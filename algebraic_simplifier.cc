#include "algebraic_simplifier.h"

#include "clang_utility_functions.h"
#include "third_party/assert_exception.h"
#include <algorithm>
#include <iostream>
#include <queue>
using namespace clang;

std::string
AlgebraicSimplifier::ast_visit_bin_op(const clang::BinaryOperator *bin_op) {
  if (can_be_simplified(bin_op)) {
    return simplify_simple_bin_op(bin_op);
  } else {
    return ast_visit_stmt(bin_op->getLHS()) +
           std::string(bin_op->getOpcodeStr()) +
           ast_visit_stmt(bin_op->getRHS());
  }
}

std::string AlgebraicSimplifier::ast_visit_cond_op(
    const clang::ConditionalOperator *cond_op) {
  if (isa<IntegerLiteral>(cond_op->getCond()) and
      dyn_cast<IntegerLiteral>(cond_op->getCond())->getValue().getSExtValue() ==
          1) {
    return ast_visit_stmt(cond_op->getTrueExpr());
  } else if (isa<IntegerLiteral>(cond_op->getCond()) and
             dyn_cast<IntegerLiteral>(cond_op->getCond())
                     ->getValue()
                     .getSExtValue() == 0) {
    return ast_visit_stmt(cond_op->getFalseExpr());
  } else {
    return ast_visit_stmt(cond_op->getCond()) + " ? " +
           ast_visit_stmt(cond_op->getTrueExpr()) + " : " +
           ast_visit_stmt(cond_op->getFalseExpr());
  }
}

bool AlgebraicSimplifier::can_be_simplified(
    const BinaryOperator *bin_op) const {
  assert_exception(bin_op);
  const auto *lhs = bin_op->getLHS();
  const auto *rhs = bin_op->getRHS();
  bool canBeSimplified = isa<IntegerLiteral>(lhs) or isa<IntegerLiteral>(rhs);
  // A BinaryOperator subclasses Expr class, so we can use
  // Expr::EvaluateAsInt to see if the given BinaryOperator object
  // reduces to a constant.
  clang::Expr::EvalResult er;
  canBeSimplified = canBeSimplified || bin_op->EvaluateAsInt(er, *(this->ctx));
  return canBeSimplified;
}

std::string AlgebraicSimplifier::simplify_simple_bin_op(
    const BinaryOperator *bin_op) const {
  assert_exception(can_be_simplified(bin_op));
  const auto opcode = bin_op->getOpcode();
  const auto *lhs = bin_op->getLHS();
  const auto *rhs = bin_op->getRHS();
  // Here lhs, rhs stand for left/right operand of a binary arithmetic
  // operation, not an assignment.

  // if RHS and LHS are constants then also calculate.
  // We use clang's built-in Expr evaluator to do this.
  // (Recall that BinaryOperator subclasses Expr, so it is
  // possible to call Expr::EvaluateAsInt directly on BinaryOperator).
  clang::Expr::EvalResult er, erLHS, erRHS;
  bool lhsIsConst, rhsIsConst;
  if (bin_op->EvaluateAsInt(er, *(this->ctx))) {
    return std::to_string(er.Val.getInt().getSExtValue());
  }

  // Simplify multiplicative identity.
  if (opcode == clang::BinaryOperatorKind::BO_Mul or
      opcode == clang::BinaryOperatorKind::BO_LAnd) {
    if (isa<IntegerLiteral>(lhs) and
        dyn_cast<IntegerLiteral>(lhs)->getValue().getSExtValue() == 1) {
      // 1 * anything = anything
      // 1 && anything = anything
      return clang_stmt_printer(rhs);
    } else if (isa<IntegerLiteral>(rhs) and
               dyn_cast<IntegerLiteral>(rhs)->getValue().getSExtValue() == 1) {
      // anything * 1 = anything
      // anything && 1 = anything
      return clang_stmt_printer(lhs);
    }
  }

  // Simplify additive identity.
  if (opcode == clang::BinaryOperatorKind::BO_Add or
      opcode == clang::BinaryOperatorKind::BO_Or) {
    if (isa<IntegerLiteral>(lhs) and
        dyn_cast<IntegerLiteral>(lhs)->getValue().getSExtValue() == 0) {
      // 0 + anything = anything
      // 0 ||  anything = anything
      return clang_stmt_printer(rhs);
    } else if (isa<IntegerLiteral>(rhs) and
               dyn_cast<IntegerLiteral>(rhs)->getValue().getSExtValue() == 0) {
      // anything + 0 = anything
      // anything || 0 = anything
      return clang_stmt_printer(lhs);
    }
  }

  // Otherwise, if LHS or RHS are constexps, we evaluate them too.
  lhsIsConst = bin_op->EvaluateAsInt(erLHS, *(this->ctx));
  rhsIsConst = bin_op->EvaluateAsInt(erRHS, *(this->ctx));
  if (lhsIsConst) {
    long long lhsVal = erLHS.Val.getInt().getSExtValue();
    if (rhsIsConst) {
      long long rhsVal = erRHS.Val.getInt().getSExtValue();
      return std::to_string(lhsVal) + std::string(bin_op->getOpcodeStr()) +
             std::to_string(rhsVal);
    } else {
      return std::to_string(lhsVal) + std::string(bin_op->getOpcodeStr()) +
             clang_stmt_printer(bin_op->getRHS());
    }
  } else {
    if (rhsIsConst) {
      long long rhsVal = erRHS.Val.getInt().getSExtValue();
      return clang_stmt_printer(bin_op->getLHS()) +
             std::string(bin_op->getOpcodeStr()) + std::to_string(rhsVal);
    }
  }

  return clang_stmt_printer(bin_op->getLHS()) +
         std::string(bin_op->getOpcodeStr()) +
         clang_stmt_printer(bin_op->getRHS());
}

