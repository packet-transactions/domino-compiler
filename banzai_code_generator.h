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
  typedef uint32_t StageId;

  /// Type of code generation: source or binary
  /// The source is useful for debugging bugs in domino's BanzaiCodeGenerator
  /// the binary is useful for running the output within banzai
  enum class CodeGenerationType { SOURCE, BINARY };

  /// Constructor taking code generation option as argument
  BanzaiCodeGenerator(const CodeGenerationType & t_code_generation_type)
    : code_generation_type_(t_code_generation_type) {}

  /// Typedef for atom positions in a pipeline
  /// Map
  /// --> from the StageId of the stage into which the atom goes.
  /// --> to all the BanzaiAtoms within that stage
  /// --> (atom order in a stage is irrelevant because they execute in parallel)
  /// We use this to lay the atoms out in a pipeline
  typedef std::map<StageId, std::vector<BanzaiAtom>> AtomPositions;

  /// Transform a translation unit into banzai atoms, useful for fuzzing
  /// initial passes in the compiler, well, everything except the last pass
  BanzaiProgram transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const;

 private:
  /// Get stage id from an atom's name.
  /// This is a hack: we are encoding the atom's position within the atom name.
  /// With the hack, BanzaiCodeGenerator can generate code for a translation unit
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

  /// Turn banzai program into a shared library,
  /// by compiling with g++ and then turning the .o file into a .so library
  BanzaiLibString gen_lib_as_string(const BanzaiProgram & banzai_program) const;

  const CodeGenerationType code_generation_type_;
};

#endif  // BANZAI_CODE_GENERATOR_H_
