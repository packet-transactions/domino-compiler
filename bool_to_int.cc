#include "bool_to_int.h"

#include <iostream>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string bool_to_int_transform(const clang::TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, bool_to_int_helper);
}

std::pair<std::string, std::vector<std::string>> bool_to_int_helper(const clang::CompoundStmt * body,
                                                                    const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair("{" + bool_to_int_stmt(body) + "}", std::vector<std::string>());
}

std::string bool_to_int_stmt(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += bool_to_int_stmt(child) + ";";
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return bool_to_int_stmt(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + bool_to_int_stmt(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    // Make sure the condition within cond_op is a MemberExpr
    assert_exception(isa<MemberExpr>(cond_op->getCond()->IgnoreParenImpCasts()));
    return     "(" + clang_stmt_printer(cond_op->getCond()) + " != 0) ? "
             + bool_to_int_stmt(cond_op->getTrueExpr()) + " : "
             + bool_to_int_stmt(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt) or isa<DeclRefExpr>(stmt) or isa<ArraySubscriptExpr>(stmt) or isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + bool_to_int_stmt(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + bool_to_int_stmt(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return bool_to_int_stmt(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = bool_to_int_stmt(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    throw std::logic_error("bool_to_int_stmt cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
