#include "pisa_code_generator.h"

#include <string>
#include <tuple>
#include <algorithm>

#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"
#include "third_party/assert_exception.h"

#include "util.h"
#include "config.h"
#include "clang_utility_functions.h"

using namespace clang;

PISACodeGenerator::StageId PISACodeGenerator::stage_id_from_name(const std::string & function_name) const {
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

int PISACodeGenerator::get_order(const Decl * decl) const {
  if (isa<VarDecl>(decl)) return 1;
  else if (isa<RecordDecl>(decl)) return 2;
  else if (isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 3;
  else if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 4;
  else if (isa<TypedefDecl>(decl)) return 5;
  else {
    throw std::logic_error("PISACodeGenerator::get_order cannot handle decl " + clang_decl_printer(decl) + " of type " + std::string(decl->getDeclKindName()));
  }}

PISACodeGenerator::PISAProgram PISACodeGenerator::transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const {
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
  std::string ret = "";

  // Storage for scalar function declarations
  std::string scalar_func_decls = "";

  // Storage for initial values of all state variables
  std::map<std::string, int> init_scalar_values;

  // Storage for initial values of all state arrays
  // TODO: We assume all elements have the same value initially.
  std::map<std::string, std::pair<PISAAtom::ArraySize, PISAAtom::ScalarValue>> init_array_values;

  // Storage for names of all packet fields for test packet generation
  PISAPacketFieldSet packet_field_set;

  // Positions for each atom.
  AtomPositions atom_defs;

  // Maximum stage_id seen so far
  StageId max_stage_id = 0;

  for (const auto * child_decl : all_decls) {
    assert_exception(child_decl);
    if (isa<VarDecl>(child_decl)) {
      const auto * var_decl = dyn_cast<VarDecl>(child_decl);
      // Forbid certain constructs
      if (not var_decl->hasInit()) {
        throw std::logic_error("All state variables must have an initializer in domino: " + clang_value_decl_printer(var_decl)+ " doesn't");
      }
      if ((not isa<IntegerLiteral>(var_decl->getInit())) and (not isa<InitListExpr>(var_decl->getInit())) and (not isa<UnaryOperator>(var_decl->getInit()))) {
        throw std::logic_error("Only integers or initializer lists can be used to initialize state variables: "
                               + clang_value_decl_printer(var_decl)
                               + " uses "
                               + clang_stmt_printer(var_decl->getInit())
                               + " of type " + std::string(var_decl->getInit()->getStmtClassName()) );
      }

      if (isa<IntegerLiteral>(var_decl->getInit()) or isa<UnaryOperator>(var_decl->getInit())) {
        init_scalar_values[clang_value_decl_printer(var_decl)] = static_cast<int>(std::stoi(clang_stmt_printer(var_decl->getInit())));
      } else {
        assert_exception(isa<InitListExpr>(var_decl->getInit()));
        const auto * underlying_type = var_decl->getType().getTypePtrOrNull();
        assert_exception(underlying_type != nullptr);
        if (not isa<ConstantArrayType>(underlying_type)) {
          throw std::logic_error("Can handle only arrays whose sizes are known at compile time");
        }
        assert_exception(dyn_cast<InitListExpr>(var_decl->getInit())->getNumInits() == 1);
        uint64_t array_size = dyn_cast<ConstantArrayType>(underlying_type)->getSize().getZExtValue();
        std::cerr << "Array declaration of array " << clang_value_decl_printer(var_decl)
                  << " with size " << array_size
                  << " and initial value " << std::stoi(clang_stmt_printer(dyn_cast<InitListExpr>(var_decl->getInit())->getInit(0)))
                  << std::endl;
        init_array_values[clang_value_decl_printer(var_decl)]  = std::make_pair(
                                                                 array_size,
                                                                 static_cast<int>(std::stoi(clang_stmt_printer(dyn_cast<InitListExpr>(var_decl->getInit())->getInit(0)))));
      }
    } else if (isa<RecordDecl>(child_decl)) {
      for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
       packet_field_set.emplace(clang_value_decl_printer(dyn_cast<ValueDecl>(field_decl)));
    } else if (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      // Pass through non-packet functions as such.
      // If there is a function body, PISA will execute them as such,
      // otherwise, it will complain with a loader error from dlsym
      scalar_func_decls += generate_scalar_func_def(dyn_cast<FunctionDecl>(child_decl));
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      assert_exception(not packet_field_set.empty());
      const auto function_name = dyn_cast<FunctionDecl>(child_decl)->getNameInfo().getName().getAsString();
      const PISAAtom pisa_atom(dyn_cast<FunctionDecl>(child_decl)->getBody(),
                                   function_name,
                                   init_scalar_values,
                                   init_array_values);

      // Add to atom_defs object, track max_stage_id so far
      const auto stage_id = stage_id_from_name(function_name);
      if (atom_defs.find(stage_id) == atom_defs.end()) {
        atom_defs[stage_id] = std::vector<PISAAtom>();
      }
      atom_defs.at(stage_id).emplace_back(pisa_atom);
      max_stage_id = std::max(max_stage_id, stage_id);
    } else {
      assert_exception(isa<TypedefDecl>(child_decl));
    }
  }

  // Add include files for PISA (the equivalent of a target ABI)
  ret += "#include \"packet.h\"\n";
  ret += "#include \"atom.h\"\n";
  ret += "#include \"pipeline.h\"\n";

  // Add an extern C flank to get around name mangling
  ret += "extern \"C\"{\n";

  // Add scalar function declarations/definitions (depending on whether the user defined them)
  ret += scalar_func_decls;

  // If atom_defs is empty, return right away
  if (atom_defs.empty()) {
    ret += "}"; // close extern C
    return ret;
  }

  // Generate all atom declarations/definitions
  for (uint32_t i = 0; i < max_stage_id + 1; i++)
    for (const auto & atom : atom_defs.at(i))
      ret += atom.get_def();

  // Put them within a pipeline
  ret += "Pipeline test_pipeline{";
  for (uint32_t i = 0; i < max_stage_id + 1; i++) {
    ret += "{";
    assert_exception(not atom_defs.at(i).empty());
    for (const auto & atom : atom_defs.at(i)) {
      ret += atom.get_name() + ",";
    }
    ret += '}';
    ret += ",";
  }
  ret += "};";

  // Close extern C declaration
  ret += "}";

  // Return PISA C++ code as such or turn it into a library
  return code_generation_type_ == CodeGenerationType::SOURCE ? ret
                                                             : gen_lib_as_string(ret);
}

PISACodeGenerator::PISALibString PISACodeGenerator::gen_lib_as_string(const PISACodeGenerator::PISAProgram & pisa_program) const {
  // TempFile to hold pisa_program
  TempFile pisa_prog_file("/tmp/pisa_prog", ".cc");
  pisa_prog_file.write(pisa_program);

  // Compile pisa_file into a .o file
  TempFile object_file("/tmp/pisa_obj", ".o");
  run({GPLUSPLUS, "-std=c++14", "-pedantic", "-Wconversion", "-Wsign-conversion", "-Wall", "-Wextra", "-Weffc++", "-Werror", "-fno-default-inline", "-g", "-c", pisa_prog_file.name(), "-fPIC", "-DPIC", "-o", object_file.name()});

  // Turn that into a shared library
  TempFile library_file("/tmp/libpisa", ".so");
  run({GPLUSPLUS, "-shared", "-o", library_file.name(), object_file.name()});

  // Return library file binary as a string
  // (hopefully doesn't bork the terminal)
  return file_to_str(library_file.name());
}
