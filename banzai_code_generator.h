#ifndef BANZAI_CODE_GENERATOR_H_
#define BANZAI_CODE_GENERATOR_H_

#include <set>
#include <string>

#include "unique_identifiers.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

/// Simple code generation for banzai, our fake pipelined switch
/// architecture that executes atoms in parallel in each stage,
/// where an atom is a function that takes a packet and returns
/// a new one, while modifying some internal state.
class BanzaiCodeGenerator {
 public:
  /// Convenience typedefs
  typedef std::string BanzaiAtomDefinition;
  typedef std::string BanzaiAtomBody;
  typedef std::set<std::string> BanzaiPacketFieldSet;
  typedef std::set<std::string> BanzaiStateVariableSet;
  typedef std::string BanzaiAtomName;
  typedef std::string BanzaiProgram;
  typedef std::string BanzaiLibString;

  /// Packet identifier when generating banzai code
  static const std::string PACKET_IDENTIFIER;

  /// State identifier when generating banzai code
  static const std::string STATE_IDENTIFIER;

  /// Transform a given clang::Stmt into banzai ops
  BanzaiAtomBody rewrite_into_banzai_ops(const clang::Stmt * stmt) const;

  /// Transform a given clang::Stmt into a banzai atom,
  /// by prepending a function signature
  /// and calling rewrite_into_banzai_ops to generate the body
  /// Return a tuple consisting of:
  /// 1. The rewritten body
  /// 2. A field set consisting of all packet fields for test packet generation
  /// 3. The name of the new atom.
  /// 4. The set of all state variables used in stmt for initialization.
  std::tuple<BanzaiAtomDefinition,
             BanzaiPacketFieldSet,
             BanzaiAtomName,
             BanzaiStateVariableSet>
  rewrite_into_banzai_atom(const clang::Stmt * stmt) const;

  /// Transform a translation unit into banzai atoms, useful for fuzzing
  /// initial passes in the compiler, well, everything except the last pass
  BanzaiProgram transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const;

 private:
  /// Get priority order of declarations in a TranslationUnitDecl.
  /// This guarantees that we process state variable definitions
  /// before packet variable declarations, before scalar functions,
  /// before packet functions.
  int get_order(const clang::Decl * decl) const;

  /// Turn banzai program into a shared library,
  /// by compiling with g++ and then turning the .o file into a .so library
  BanzaiLibString gen_lib_as_string(const BanzaiProgram & banzai_program) const;

  /// Generate unique names for atom functions.
  /// The only forbidden identifiers are PACKET_IDENTIFIER and STATE_IDENTIFIER,
  /// because they are used as parameter variables for the atoms.
  UniqueIdentifiers unique_identifiers_ = UniqueIdentifiers(std::set<std::string>{PACKET_IDENTIFIER, STATE_IDENTIFIER});
};

#endif  // BANZAI_CODE_GENERATOR_H_
