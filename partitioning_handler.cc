#include <map>
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

  std::map<const BinaryOperator *, std::vector<const BinaryOperator *>> succ_graph;
  std::map<const BinaryOperator *, std::vector<const BinaryOperator *>> pred_graph;
  for (const auto & op : useful_ops) {
    succ_graph[op] = {};
    pred_graph[op] = {};
  }

  for (uint32_t i = 0; i < useful_ops.size(); i++) {
    for (uint32_t j = i + 1; j < useful_ops.size(); j++) {
      if (depends(useful_ops.at(i), useful_ops.at(j))) {
        // edge from i ---> j
        succ_graph.at(useful_ops.at(i)).emplace_back(useful_ops.at(j));
        pred_graph.at(useful_ops.at(j)).emplace_back(useful_ops.at(i));
      }
    }
  }
}

bool PartitioningHandler::op_reads_var(const BinaryOperator * op, const DeclRefExpr * var) const {
  // This is an ugly hack using string search (to say the least!)
  // TODO: Fix this at some later point in time once it is clear how to do it.
  return (clang_stmt_printer(op).find(clang_stmt_printer(var)) != std::string::npos);
}

bool PartitioningHandler::depends(const BinaryOperator * op1, const BinaryOperator * op2) const {
  // assume and check that op1 precedes op2 in program order
  assert(op1->getLocStart() < op2->getLocStart());
  assert(isa<DeclRefExpr>(op1->getLHS()));
  assert(isa<DeclRefExpr>(op2->getLHS()));

  // op1 writes a variable (LHS) that op2 reads. (Read After Write)
  if (op_reads_var(op2, dyn_cast<DeclRefExpr>(op1->getLHS()))) {
    return true;
  }

  // op1 writes the same variable that op2 writes (Write After Write)
  if (clang_stmt_printer(op1->getLHS()) == clang_stmt_printer(op2->getLHS())) {
    return true;
  }

  // op1 reads a variable that op2 writes (Write After Read)
  if (op_reads_var(op1, dyn_cast<DeclRefExpr>(op2->getLHS()))) {
    return true;
  }

  return false;
}
