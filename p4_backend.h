#ifndef P4_CODE_GENERATOR_H_
#define P4_CODE_GENERATOR_H_

#include <set>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

#include "p4_atom.h"
#include "unique_identifiers.h"

/// Simple code generation for p4, our fake pipelined switch
/// architecture that executes atoms in parallel in each stage,
/// where an atom is a function that takes a packet and returns
/// a new one, while modifying some internal state.
class P4CodeGenerator {
 public:
  /// Convenience typedefs
  typedef std::set<std::string> P4PacketFieldSet;
  typedef std::string P4Program;
  typedef std::string P4LibString;
  typedef uint32_t StageId;

  /// Typedef for atom positions in a pipeline
  /// Map
  /// --> from the StageId of the stage into which the atom goes.
  /// --> to all the P4Atoms within that stage
  /// --> (atom order in a stage is irrelevant because they execute in parallel)
  /// We use this to lay the atoms out in a pipeline
  typedef std::map<StageId, std::vector<P4Atom>> AtomPositions;

  /// Transform a translation unit into p4 atoms, useful for fuzzing
  /// initial passes in the compiler, well, everything except the last pass
  P4Program transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const;

 private:
  /// Get stage id from an atom's name.
  /// This is a hack: we are encoding the atom's position within the atom name.
  /// With the hack, P4CodeGenerator can generate code for a translation unit
  /// with more than one packet function.
  /// Until the last pass, there is only 1 packet function in a translation unit.
  /// But after the last pass, partitioning, there are multiple functions,
  /// one for each atom, and we need to somehow encode the atom's position
  /// into the function name to construct the test_pipeline object.
  StageId stage_id_from_name(const std::string & function_name) const;

  /// Get priority order of declarations in a TranslationUnitDecl.
  /// This guarantees that we process state variable definitions
  /// before packet variable declarations, before scalar functions,
  /// before packet functions.
  int get_order(const clang::Decl * decl) const;

};

#endif  // P4_CODE_GENERATOR_H_
