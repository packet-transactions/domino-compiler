#ifndef PARTITIONING_HANDLER_H_
#define PARTITIONING_HANDLER_H_

#include <string>
#include <set>
#include "clang/Lex/Lexer.h"
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

class PartitioningHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  PartitioningHandler() {};

  /// Callback whenever there's a match
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;

 private:
  /// Convenience typedef: A vector of instructions, in our hyper-restrictive
  /// language everything is a BinaryOperator
  typedef std::vector<const clang::BinaryOperator*> InstructionVector;

  /// Convenience typedef: A vector of vectors representing instructions
  /// that go into each pipeline stage
  typedef std::vector<InstructionVector> InstructionPartitioning;

  /// Does operation read variable?
  bool op_reads_var(const clang::BinaryOperator * op, const clang::DeclRefExpr * var) const;

  /// Is there a dependence _from_ BinaryOperator op1 _to_ BinaryOperator op2?
  /// Returns true if op1 MUST precede op 2
  bool depends(const clang::BinaryOperator * op1, const clang::BinaryOperator * op2) const;

  /// Core partitioning logic to generate a pipeline
  /// Build a dag of dependencies and then schedule using the DAG
  InstructionPartitioning partition_into_pipeline(const InstructionVector & inst_vector) const;

  /// Get all stateful writes from a BinaryOperator
  std::vector<std::string> get_stateful_writes(const clang::BinaryOperator * inst) const;

  /// Get all stateful reads from a BinaryOperator
  std::vector<std::string> get_stateful_reads(const clang::BinaryOperator * inst) const;

  /// Check for pipeline-wide stateful variables
  /// Return a vector of strings that represent all
  /// variables that are pipeline-wide, and _require_ recirculation,
  /// in the absence of packed-word instructions.
  std::vector<std::string> check_for_pipeline_vars(const InstructionPartitioning & partitioning) const;
};

#endif  // PARTITIONING_HANDLER_H_
