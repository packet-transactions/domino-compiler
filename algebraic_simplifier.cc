#include "algebraic_simplifier.h"

#include <iostream>

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

using namespace clang;

std::string algebraic_simplifier_transform(const clang::TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, algebraic_simplify_helper);
}

std::pair<std::string, std::vector<std::string>> algebraic_simplify_helper(const clang::CompoundStmt * body,
                                                                           const std::string & pkt_name __attribute__((unused))) {
  return std::make_pair("{" + algebraic_simplify_stmt(body) + "}", std::vector<std::string>());
}

static bool can_be_simplified(const BinaryOperator * bin_op) {
  assert_exception(bin_op);
  const auto * lhs = bin_op->getLHS();
  const auto * rhs = bin_op->getRHS();
  return isa<IntegerLiteral>(lhs) or isa<IntegerLiteral>(rhs);
}

// Simplify a bin op where the LHS and RHS are both simple
// i.e. IntegerLiteral, MemberExpr, or DeclRefExpr
static std::string simplify_simple_bin_op(const BinaryOperator * bin_op) {
  assert_exception(can_be_simplified(bin_op));
  const auto opcode = bin_op->getOpcode();
  const auto * lhs = bin_op->getLHS();
  const auto * rhs = bin_op->getRHS();
  if (opcode == clang::BinaryOperatorKind::BO_Mul or opcode == clang::BinaryOperatorKind::BO_LAnd) {
    if (isa<IntegerLiteral>(lhs) and dyn_cast<IntegerLiteral>(lhs)->getValue().getSExtValue() == 1) {
      // 1 * anything = anything
      // 1 && anything = anything
      return clang_stmt_printer(rhs);
    } else if (isa<IntegerLiteral>(rhs) and dyn_cast<IntegerLiteral>(rhs)->getValue().getSExtValue() == 1) {
      // anything * 1 = anything
      // anything && 1 = anything
      return clang_stmt_printer(lhs);
    }
  }

  if (opcode == clang::BinaryOperatorKind::BO_Add or opcode == clang::BinaryOperatorKind::BO_Or) {
    if (isa<IntegerLiteral>(lhs) and dyn_cast<IntegerLiteral>(lhs)->getValue().getSExtValue() == 0) {
      // 0 + anything = anything
      // 0 ||  anything = anything
      return clang_stmt_printer(rhs);
    } else if (isa<IntegerLiteral>(rhs) and dyn_cast<IntegerLiteral>(rhs)->getValue().getSExtValue() == 0) {
      // anything + 0 = anything
      // anything || 0 = anything
      return clang_stmt_printer(lhs);
    }
  }

  return clang_stmt_printer(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + clang_stmt_printer(bin_op->getRHS());
}

std::string algebraic_simplify_stmt(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += algebraic_simplify_stmt(child) + ";";
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    if (can_be_simplified(bin_op)) {
      return simplify_simple_bin_op(bin_op);
    } else {
      return algebraic_simplify_stmt(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + algebraic_simplify_stmt(bin_op->getRHS());
    }
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    if (isa<IntegerLiteral>(cond_op->getCond()) and dyn_cast<IntegerLiteral>(cond_op->getCond())->getValue().getSExtValue() == 1) {
      return algebraic_simplify_stmt(cond_op->getTrueExpr());
    } else if (isa<IntegerLiteral>(cond_op->getCond()) and dyn_cast<IntegerLiteral>(cond_op->getCond())->getValue().getSExtValue() == 1) {
      return algebraic_simplify_stmt(cond_op->getFalseExpr());
    } else {
      return   "(" + algebraic_simplify_stmt(cond_op->getCond()) + ") ? ("
               + algebraic_simplify_stmt(cond_op->getTrueExpr()) + ") : ("
               + algebraic_simplify_stmt(cond_op->getFalseExpr()) + ")";
    }
  } else if (isa<MemberExpr>(stmt) or isa<DeclRefExpr>(stmt) or isa<ArraySubscriptExpr>(stmt) or isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + algebraic_simplify_stmt(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + algebraic_simplify_stmt(un_op->getSubExpr());
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return algebraic_simplify_stmt(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = algebraic_simplify_stmt(child);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else {
    throw std::logic_error("algebraic_simplify_stmt cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
