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
  /// Packet identifier when generating banzai code
  static const std::string PACKET_IDENTIFIER;

  /// State identifier when generating banzai code
  static const std::string STATE_IDENTIFIER;

  /// Transform a given clang::Stmt into banzai ops
  std::string rewrite_into_banzai_ops(const clang::Stmt * stmt) const;

  /// Transform a given clang::Stmt into a banzai atom,
  /// by prepending a function signature, appending a return Pkt
  /// and calling rewrite_into_banzai_ops to generate the body
  std::string rewrite_into_banzai_atom(const clang::Stmt * stmt) const;

  /// Transform a translation unit into banzai atoms, useful for fuzzing
  /// initial passes in the compiler, well, everything except the last pass
  std::string transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const;

  /// Determine all fields used within a clang::Stmt, to generate a field
  /// list for banzai. The field list is to generate random packets.
  std::set<std::string> gen_pkt_field_list(const clang::Stmt * stmt) const;

 private:
  /// Generate unique names for atom functions.
  /// The only forbidden identifiers are PACKET_IDENTIFIER and STATE_IDENTIFIER,
  /// because they are used as parameter variables for the atoms.
  UniqueIdentifiers unique_identifiers_ = UniqueIdentifiers(std::set<std::string>{PACKET_IDENTIFIER, STATE_IDENTIFIER});
};

#endif  // BANZAI_CODE_GENERATOR_H_
