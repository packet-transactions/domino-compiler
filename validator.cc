#include "validator.h"

#include <iostream>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string validator_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, validator_helper);
}

std::pair<std::string, std::vector<std::string>> validator_helper(const clang::CompoundStmt * body,
                                                                  const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair("{" + validator(body) + "}", std::vector<std::string>());
}

std::string validator(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += validator(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + validator(if_stmt->getCond()) + ") {" + validator(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + validator(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<CompoundAssignOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return validator(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + validator(bin_op->getRHS());
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return validator(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + validator(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return     validator(cond_op->getCond()) + " ? "
             + validator(cond_op->getTrueExpr()) + " : "
             + validator(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt) or isa<DeclRefExpr>(stmt) or isa<ArraySubscriptExpr>(stmt) or isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + validator(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + validator(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return validator(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = validator(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    throw std::logic_error("validator error: the statement\n"
                           + clang_stmt_printer(stmt)
                           + "\nis of type "
                           + std::string(stmt->getStmtClassName())
                           + ", which isn't allowed in domino");
  }
}
