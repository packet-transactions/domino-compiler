#include <iostream>
#include "partitioning_handler.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

void PartitioningHandler::run(const MatchFinder::MatchResult & t_result) {
  const FunctionDecl *function_decl_expr = t_result.Nodes.getNodeAs<clang::FunctionDecl>("functionDecl");
  assert(function_decl_expr != nullptr);
  assert(isa<CompoundStmt>(function_decl_expr->getBody()));
  std::vector<const BinaryOperator *> useful_ops;
  for (const auto & child : function_decl_expr->getBody()->children()) {
    assert(child);
    assert(isa<DeclStmt>(child) or isa<BinaryOperator>(child));
    if (isa<BinaryOperator>(child)) {
      const auto * bin_op = dyn_cast<BinaryOperator>(child);
      assert(bin_op->isAssignmentOp());
      assert(isa<DeclRefExpr>(bin_op->getLHS()));
      useful_ops.push_back(bin_op);
    }
  }

  for (const auto & op1 : useful_ops) {
    for (const auto & op2 : useful_ops) {
      assert(op1->isAssignmentOp());
      assert(op2->isAssignmentOp());
    }
  }
}
