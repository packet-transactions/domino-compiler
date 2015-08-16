#include "banzai_code_generator.h"

#include <string>
#include <tuple>
#include <algorithm>

#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"

#include "util.h"
#include "config.h"
#include "clang_utility_functions.h"

using namespace clang;

BanzaiCodeGenerator::StageId BanzaiCodeGenerator::stage_id_from_name(const std::string & function_name) const {
  // This is the place where we wrap all the encoding magic that lets us go
  // from a function name to its position within a pipeline
  if (function_name[0] != '_') {
    // If function doesn't start with _, it has no special meaning, return stage_id of 0
    return 0;
  } else {
    std::vector<std::string> split_strings = split(function_name, "_");
    if (split_strings.size() != 4) { // Contents of split_strings should be { "", "atom", stage_id, atom_id }
      throw std::logic_error("Special function names should be of the form _xyz_#_#, found function with name " + function_name);
    } else {
      return static_cast<uint32_t>(std::stoul(split_strings[2]));
    }
  }
}

int BanzaiCodeGenerator::get_order(const Decl * decl) const {
  if (isa<VarDecl>(decl)) return 1;
  else if (isa<RecordDecl>(decl)) return 2;
  else if (isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 3;
  else if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 4;
  else if (isa<TypedefDecl>(decl)) return 5;
  else {assert(false); return -1; }
}

BanzaiCodeGenerator::BanzaiProgram BanzaiCodeGenerator::transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const {
  // Accumulate all declarations
  std::vector<const Decl*> all_decls;
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls())
    all_decls.emplace_back(decl);

  // Sort all_decls
  std::sort(all_decls.begin(),
            all_decls.end(),
            [this] (const auto * decl1, const auto * decl2)
            { return this->get_order(decl1) < this->get_order(decl2); });

  // Storage for returned string
  std::string ret;

  // Storage for initial values of all state variables
  std::map<std::string, uint32_t> init_values;

  // Storage for names of all packet fields for test packet generation
  BanzaiPacketFieldSet packet_field_set;

  // Positions for each atom.
  AtomPositions atom_defs;

  // Maximum stage_id seen so far
  StageId max_stage_id = 0;

  for (const auto * child_decl : all_decls) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl)) {
      const auto * var_decl = dyn_cast<VarDecl>(child_decl);
      // Forbid certain constructs
      if (not var_decl->hasInit()) throw std::logic_error("All state variables must have an initializer in domino: " + clang_value_decl_printer(var_decl)+ " doesn't");
      if (init_values.find(clang_value_decl_printer(var_decl)) != init_values.end()) throw std::logic_error("Reinitializing " + clang_value_decl_printer(var_decl) + " to " + clang_stmt_printer(var_decl->getInit()) + " not permitted");
      if (not isa<IntegerLiteral>(var_decl->getInit())) throw std::logic_error("Only integers can be used to initialize state variables: " + clang_value_decl_printer(var_decl) + " uses " + clang_stmt_printer(var_decl->getInit()));
      init_values[clang_value_decl_printer(var_decl)] = static_cast<uint32_t>(std::stoul(clang_stmt_printer(var_decl->getInit())));
    } else if (isa<RecordDecl>(child_decl)) {
      for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
       packet_field_set.emplace(clang_value_decl_printer(dyn_cast<ValueDecl>(field_decl)));
    } else if (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      // Just quench these, don't emit them
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      assert(not packet_field_set.empty());
      const auto function_name = dyn_cast<FunctionDecl>(child_decl)->getNameInfo().getName().getAsString();
      const BanzaiAtom banzai_atom(dyn_cast<FunctionDecl>(child_decl)->getBody(),
                                   function_name,
                                   init_values);

      // Add to atom_defs object, track max_stage_id so far
      const auto stage_id = stage_id_from_name(function_name);
      if (atom_defs.find(stage_id) == atom_defs.end()) {
        atom_defs[stage_id] = std::vector<BanzaiAtom>();
      }
      atom_defs.at(stage_id).emplace_back(banzai_atom);
      max_stage_id = std::max(max_stage_id, stage_id);
    } else {
      assert(isa<TypedefDecl>(child_decl));
    }
  }

  // Add include files for banzai (the equivalent of a target ABI)
  ret += "#include \"packet.h\"\n";
  ret += "#include \"atom.h\"\n";
  ret += "#include \"pipeline.h\"\n";

  // Add an extern C flank to get around name mangling
  ret += "extern \"C\"{\n";

  // Generate all atom declarations/definitions
  for (uint32_t i = 0; i < max_stage_id + 1; i++)
    for (const auto & atom : atom_defs.at(i))
      ret += atom.get_def() + ";";

  // Put them within a pipeline
  ret += "Pipeline test_pipeline{";
  for (uint32_t i = 0; i < max_stage_id + 1; i++) {
    ret += "{";
    assert(not atom_defs.at(i).empty());
    for (const auto & atom : atom_defs.at(i)) {
      ret += atom.get_name() + ",";
    }
    ret.back() = '}';
  }
  ret += "};";

  // Close extern C declaration
  ret += "}";

  // Turn banzai C++ code into a library
  return gen_lib_as_string(ret);
}

BanzaiCodeGenerator::BanzaiLibString BanzaiCodeGenerator::gen_lib_as_string(const BanzaiCodeGenerator::BanzaiProgram & banzai_program) const {
  // TempFile to hold banzai_program
  TempFile banzai_prog_file("/tmp/banzai_prog", ".cc");
  banzai_prog_file.write(banzai_program);

  // Compile banzai_file into a .o file
  TempFile object_file("/tmp/banzai_obj", ".o");
  run({GPLUSPLUS, "-std=c++14", "-pedantic", "-Wconversion", "-Wsign-conversion", "-Wall", "-Wextra", "-Weffc++", "-Werror", "-fno-default-inline", "-g", "-c", banzai_prog_file.name(), "-fPIC", "-DPIC", "-o", object_file.name()});

  // Turn that into a shared library
  TempFile library_file("/tmp/libbanzai", ".so");
  run({GPLUSPLUS, "-shared", "-o", library_file.name(), object_file.name()});

  // Return library file binary as a string
  // (hopefully doesn't bork the terminal)
  return file_to_str(library_file.name());
}
