#ifndef ALGEBRAIC_SIMPLIFIER_H_
#define ALGEBRAIC_SIMPLIFIER_H_

#include "ast_visitor.h"

class AlgebraicSimplifier : public AstVisitor {
 protected:
  /// Simplify a binary operator using
  /// algebraic rewrite rules.
  std::string ast_visit_bin_op(const clang::BinaryOperator * bin_op) override;

  /// Simplify a conditional operator using
  /// algebraic rewrite rules.
  std::string ast_visit_cond_op(const clang::ConditionalOperator * cond_op) override;

 private:
  /// Check whether a bin_op can be simplified
  /// i.e. one of its arguments is a constant
  bool can_be_simplified(const clang::BinaryOperator * bin_op) const;

  /// Simplify a bin op where the LHS and RHS are both simple
// i.e. IntegerLiteral, MemberExpr, or DeclRefExpr
  std::string simplify_simple_bin_op(const clang::BinaryOperator * bin_op) const;
};

#endif // ALGEBRAIC_SIMPLIFIER_H_
