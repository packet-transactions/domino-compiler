#ifndef BOOL_TO_INT_H_
#define BOOL_TO_INT_H_

#include "ast_visitor.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

/// Convert bools of the form pkt.tmp ? x : y
/// to (pkt.tmp != 0) ? x = y
class BoolToInt : public AstVisitor {
 protected:
  /// Touch only conditonal operators, leave the
  /// rest to the base class AstVisitor
  std::string ast_visit_cond_op(const clang::ConditionalOperator * cond_op) override {
    assert_exception(cond_op);
    // Make sure the condition within cond_op is a MemberExpr
    assert_exception(clang::isa<clang::MemberExpr>(cond_op->getCond()->IgnoreParenImpCasts()));
    return     "(" + clang_stmt_printer(cond_op->getCond()) + " != 0) ? "
             + ast_visit_stmt(cond_op->getTrueExpr()) + " : "
             + ast_visit_stmt(cond_op->getFalseExpr());
  }
};

#endif // BOOL_TO_INT_H_
