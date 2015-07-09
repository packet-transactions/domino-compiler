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
  /// Does operation read variable?
  bool op_reads_var(const clang::BinaryOperator * op, const clang::DeclRefExpr * var) const;

  /// Is there a dependence _from_ BinaryOperator op1 _to_ BinaryOperator op2?
  /// Returns true if op1 MUST precede op 2
  bool depends(const clang::BinaryOperator * op1, const clang::BinaryOperator * op2) const;
};

#endif  // PARTITIONING_HANDLER_H_
