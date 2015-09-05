#include "redundancy_remover.h"

#include <iostream>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string redundancy_remover_transform(const clang::TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, redundancy_remover_helper);
}

std::pair<std::string, std::vector<std::string>> redundancy_remover_helper(const clang::CompoundStmt * body,
                                                                           const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair(redundancy_remover_stmt(body), std::vector<std::string>());
}

std::string redundancy_remover_stmt(const Stmt * stmt) {
  assert_exception(stmt);
  if(isa<CompoundStmt>(stmt)) {
    const auto * comp_stmt = dyn_cast<CompoundStmt>(stmt);

    // Check if you nest exactly one CompoundStmt
    if (comp_stmt->size() == 1 and isa<CompoundStmt>(comp_stmt->body_back())) {
      std::cerr <<  "Nesting exactly one CompoundStmt" << std::endl;
      return clang_stmt_printer(comp_stmt->body_back()); // My version of libclang doesn't have a body_front()
    } else {
      std::string ret;
      for (const auto & stmt : comp_stmt->body()) {
        ret += redundancy_remover_stmt(stmt) + ";";
      }
      return ret;
    }
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return    redundancy_remover_stmt(cond_op->getCond()) + " ? "
            + redundancy_remover_stmt(cond_op->getTrueExpr()) + " : "
            + redundancy_remover_stmt(cond_op->getFalseExpr());
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return redundancy_remover_stmt(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + redundancy_remover_stmt(bin_op->getRHS());
  } else if (isa<MemberExpr>(stmt) or isa<DeclRefExpr>(stmt) or isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    const auto * array_op = dyn_cast<ArraySubscriptExpr>(stmt);
    return redundancy_remover_stmt(array_op->getBase()) + "[" + redundancy_remover_stmt(array_op->getIdx()) + "]";
  } else if (isa<ParenExpr>(stmt)) {
    const auto * paren_expr = dyn_cast<ParenExpr>(stmt);
    if (isa<ParenExpr>(paren_expr->getSubExpr())) return clang_stmt_printer(paren_expr->getSubExpr());
    else return "(" + redundancy_remover_stmt(paren_expr->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + redundancy_remover_stmt(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return redundancy_remover_stmt(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = redundancy_remover_stmt(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    throw std::logic_error("redundancy_remover_stmt cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
