#include "banzai_code_generator.h"

#include <string>
#include <tuple>

#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"

#include "util.h"
#include "config.h"
#include "clang_utility_functions.h"

using namespace clang;

int BanzaiCodeGenerator::get_order(const Decl * decl) const {
  if (isa<VarDecl>(decl)) return 1;
  else if (isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 2;
  else if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 3;
  else if (isa<RecordDecl>(decl)) return 4;
  else if (isa<TypedefDecl>(decl)) return 5;
  else {assert(false); return -1; }
}

std::string BanzaiCodeGenerator::test_fields_decl(const BanzaiCodeGenerator::BanzaiPacketFieldSet & packet_field_set) const {
  // Generate test_fields for banzai
  std::string ret = "PacketFieldSet test_fields(";
  if (not packet_field_set.empty()) {
    ret += "{";
    for (const auto & field : packet_field_set) {
      ret += "\"" + field + "\",";
    }
    ret.back() = '}';
  } else {
    ret += "{}";
  }
  ret += ");";
  return ret;
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
      const auto function_name = dyn_cast<FunctionDecl>(child_decl)->getNameInfo().getName().getAsString();
      const BanzaiAtom banzai_atom(dyn_cast<FunctionDecl>(child_decl)->getBody(),
                                   function_name,
                                   init_values);

      // Add include files for banzai (the equivalent of a target ABI)
      ret += "#include \"packet.h\"\n";
      ret += "#include \"atom.h\"\n";
      ret += "#include \"pipeline.h\"\n";

      // Add an extern C flank to get around name mangling
      ret += "extern \"C\"{\n";

      // Generate atom definition
      ret += banzai_atom.get_def();

      // Generate test_fields for banzai
      ret += test_fields_decl(packet_field_set);

      // Generate test_pipeline for banzai
      ret += "Pipeline test_pipeline{{" + banzai_atom.get_name() + "}};";

      // Close extern C declaration
      ret += "}";
    } else {
      assert(isa<TypedefDecl>(child_decl));
    }
  }

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
