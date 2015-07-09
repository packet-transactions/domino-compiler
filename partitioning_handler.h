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

  /// Convenience typedef: A vector of vectors representing instructions scheduled
  /// at each time slot or clock
  typedef std::vector<InstructionVector> InstructionSchedule;

  /// Does operation read variable?
  bool op_reads_var(const clang::BinaryOperator * op, const clang::DeclRefExpr * var) const;

  /// Is there a dependence _from_ BinaryOperator op1 _to_ BinaryOperator op2?
  /// Returns true if op1 MUST precede op 2
  bool depends(const clang::BinaryOperator * op1, const clang::BinaryOperator * op2) const;

  /// Core partitioning logic to generate a pipeline
  /// Build a dag of dependencies and then schedule using the DAG
  InstructionSchedule partition_into_pipeline(const InstructionVector & inst_vector) const;
};

#endif  // PARTITIONING_HANDLER_H_
