#include "array_validator.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string array_validator_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, array_validate_body);
}

std::pair<std::string, std::vector<std::string>> array_validate_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused))) {
  const auto check = check_array_subscript_expr(function_body);
  assert_exception(check);
  return std::make_pair(clang_stmt_printer(function_body), std::vector<std::string>());
}

bool check_array_subscript_expr(const Stmt * stmt) {
  // Recursively scan stmt to check that
  // array subscripts are always a MemberRefExpr
  assert_exception(stmt);
  if (isa<CompoundStmt>(stmt)) {
    bool ret = true;
    for (const auto & child : stmt->children()) {
      ret = ret and check_array_subscript_expr(child);
    }
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    if (if_stmt->getElse() != nullptr) {
      return check_array_subscript_expr(if_stmt->getCond())
             and check_array_subscript_expr(if_stmt->getThen())
             and check_array_subscript_expr(if_stmt->getElse());
    } else {
      return check_array_subscript_expr(if_stmt->getCond())
             and check_array_subscript_expr(if_stmt->getThen());
    }
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return check_array_subscript_expr(bin_op->getLHS())
           and check_array_subscript_expr(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return check_array_subscript_expr(cond_op->getCond())
           and check_array_subscript_expr(cond_op->getTrueExpr())
           and check_array_subscript_expr(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt)) {
    return true;
  } else if (isa<DeclRefExpr>(stmt)) {
    return true;
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    const auto * array_op = dyn_cast<ArraySubscriptExpr>(stmt);
    assert_exception(isa<ImplicitCastExpr>(array_op->getIdx()));
    bool check = isa<MemberExpr>(dyn_cast<ImplicitCastExpr>(array_op->getIdx())->getSubExpr());
    if (check == false) {
      throw std::logic_error("Only packet fields are allowed as array indices.\n"
                             "The expression " + clang_stmt_printer(array_op) + "\n" +
                             "uses an index "  + clang_stmt_printer(array_op->getIdx()) + "\n" +
                             "of type " + std::string(array_op->getIdx()->getStmtClassName()));
    }
    return check;
  } else if (isa<IntegerLiteral>(stmt)) {
    return true;
  } else if (isa<ParenExpr>(stmt)) {
    return check_array_subscript_expr(dyn_cast<ParenExpr>(stmt)->getSubExpr());
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return check_array_subscript_expr(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return true;
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    bool ret = true;
    for (const auto * child : call_expr->arguments()) {
      const auto child_uses = check_array_subscript_expr(child);
      ret = ret and child_uses;
    }
    return ret;
  } else {
    throw std::logic_error("check_array_subscript_expr cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
