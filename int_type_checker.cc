#include "int_type_checker.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"


using namespace clang;

std::string int_type_checker_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, int_type_check_body);
}

std::pair<std::string, std::vector<std::string>> int_type_check_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused))) {
  check_for_implicit_casts(function_body);
  return std::make_pair(clang_stmt_printer(function_body), std::vector<std::string>());
}

bool check_for_implicit_casts(const Stmt * stmt) {
  // Recursively scan stmt to check for implicit casts
  assert_exception(stmt);
  if (isa<CompoundStmt>(stmt)) {
    bool ret = false;
    for (const auto & child : stmt->children()) {
      ret = ret or check_for_implicit_casts(child);
    }
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    if (if_stmt->getElse() != nullptr) {
      return check_for_implicit_casts(if_stmt->getCond())
             or check_for_implicit_casts(if_stmt->getThen())
             or check_for_implicit_casts(if_stmt->getElse());
    } else {
      return check_for_implicit_casts(if_stmt->getCond())
             or check_for_implicit_casts(if_stmt->getThen());
    }
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return check_for_implicit_casts(bin_op->getLHS())
           or check_for_implicit_casts(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return check_for_implicit_casts(cond_op->getCond())
           or check_for_implicit_casts(cond_op->getTrueExpr())
           or check_for_implicit_casts(cond_op->getFalseExpr());
  } else if (isa<MemberExpr>(stmt)) {
    return false;
  } else if (isa<DeclRefExpr>(stmt)) {
    return false;
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    return false;
  } else if (isa<IntegerLiteral>(stmt)) {
    return false;
  } else if (isa<ParenExpr>(stmt)) {
    return check_for_implicit_casts(dyn_cast<ParenExpr>(stmt)->getSubExpr());
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return check_for_implicit_casts(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    const auto * cast_expr = dyn_cast<ImplicitCastExpr>(stmt);
    bool check = cast_expr->getCastKind() == CastKind::CK_IntegralCast
                 or cast_expr->getCastKind() == CastKind::CK_IntegralToBoolean;
    if (check == true) {
      throw std::logic_error("Found ImplicitCastExpr with a cast of type " +
                             std::string(cast_expr->getCastKindName()) +
                             " in expression " +
                             clang_stmt_printer(cast_expr));
    }
    return check;
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    bool ret = false;
    for (const auto * child : call_expr->arguments()) {
      const auto child_uses = check_for_implicit_casts(child);
      ret = ret or child_uses;
    }
    return ret;
  } else {
    throw std::logic_error("check_for_implicit_casts cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
