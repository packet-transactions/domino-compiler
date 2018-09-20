#include "algebraic_simplifier.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

using namespace clang;

std::string AlgebraicSimplifier::ast_visit_bin_op(const clang::BinaryOperator * bin_op) {
  if (can_be_simplified(bin_op)) {
    return simplify_simple_bin_op(bin_op);
  } else {
    return ast_visit(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + ast_visit(bin_op->getRHS());
  }
}

std::string AlgebraicSimplifier::ast_visit_cond_op(const clang::ConditionalOperator * cond_op) {
  if (isa<IntegerLiteral>(cond_op->getCond()) and dyn_cast<IntegerLiteral>(cond_op->getCond())->getValue().getSExtValue() == 1) {
    return ast_visit(cond_op->getTrueExpr());
  } else if (isa<IntegerLiteral>(cond_op->getCond()) and dyn_cast<IntegerLiteral>(cond_op->getCond())->getValue().getSExtValue() == 0) {
    return ast_visit(cond_op->getFalseExpr());
  } else {
    return     ast_visit(cond_op->getCond()) + " ? "
             + ast_visit(cond_op->getTrueExpr()) + " : "
             + ast_visit(cond_op->getFalseExpr());
  }
}

bool AlgebraicSimplifier::can_be_simplified(const BinaryOperator * bin_op) const {
  assert_exception(bin_op);
  const auto * lhs = bin_op->getLHS();
  const auto * rhs = bin_op->getRHS();
  return isa<IntegerLiteral>(lhs) or isa<IntegerLiteral>(rhs);
}

std::string AlgebraicSimplifier::simplify_simple_bin_op(const BinaryOperator * bin_op) const {
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
