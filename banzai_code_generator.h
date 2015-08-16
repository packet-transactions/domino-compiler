#ifndef BANZAI_CODE_GENERATOR_H_
#define BANZAI_CODE_GENERATOR_H_

#include <set>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

#include "banzai_atom.h"
#include "unique_identifiers.h"

/// Simple code generation for banzai, our fake pipelined switch
/// architecture that executes atoms in parallel in each stage,
/// where an atom is a function that takes a packet and returns
/// a new one, while modifying some internal state.
class BanzaiCodeGenerator {
 public:
  /// Convenience typedefs
  typedef std::set<std::string> BanzaiPacketFieldSet;
  typedef std::string BanzaiProgram;
  typedef std::string BanzaiLibString;

  /// Transform a translation unit into banzai atoms, useful for fuzzing
  /// initial passes in the compiler, well, everything except the last pass
  BanzaiProgram transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const;

 private:
  /// Get priority order of declarations in a TranslationUnitDecl.
  /// This guarantees that we process state variable definitions
  /// before packet variable declarations, before scalar functions,
  /// before packet functions.
  int get_order(const clang::Decl * decl) const;

  /// Turn packet_field_set into a string that represents
  /// C++ code to instantiate a packet_field_set in Banzai
  std::string test_fields_decl(const BanzaiPacketFieldSet & packet_field_set) const;

  /// Turn banzai program into a shared library,
  /// by compiling with g++ and then turning the .o file into a .so library
  BanzaiLibString gen_lib_as_string(const BanzaiProgram & banzai_program) const;
};

#endif  // BANZAI_CODE_GENERATOR_H_
