#ifndef ARRAY_VALIDATOR_H_
#define ARRAY_VALIDATOR_H_

#include "ast_visitor.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

/// Check that an array expression for a particular array
/// is indexed only by a MemberRefExpr (a packet field).
/// TODO: Also check that this MemberRefExpr isn't ever reassigned.
/// We need to do this soon because otherwise, stateful_flanks will
/// end up with painful input that it won't know how to handle.
class ArrayValidator : public AstVisitor {
 protected:
  /// Touch only array subscript operator
  std::string ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr) override {
    assert_exception(array_subscript_expr);
    assert_exception(clang::isa<clang::ImplicitCastExpr>(array_subscript_expr->getIdx()));
    bool check = clang::isa<clang::MemberExpr>(clang::dyn_cast<clang::ImplicitCastExpr>(array_subscript_expr->getIdx())->getSubExpr());
    if (check == false) {
      throw std::logic_error("Only packet fields are allowed as array indices.\n"
                             "The expression " + clang_stmt_printer(array_subscript_expr) + "\n" +
                             "uses an index "  + clang_stmt_printer(array_subscript_expr->getIdx()) + "\n" +
                             "of type " + std::string(array_subscript_expr->getIdx()->getStmtClassName()));
    } else {
      // Delegate to base class
      return AstVisitor::ast_visit_array_subscript_expr(array_subscript_expr);
    }
  }
};

#endif  // ARRAY_VALIDATOR_H_
