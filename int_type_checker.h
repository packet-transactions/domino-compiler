#ifndef INT_TYPE_CHECKER_H_
#define INT_TYPE_CHECKER_H_

#include "ast_visitor.h"

#include "clang/AST/Expr.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

class IntTypeChecker : public AstVisitor {
 protected:
  /// Checks that there are no ImplicitCastExprs that invoke an IntegralCast
  /// (in some ways, a poor man's version of Wconversion and Wsign-conversion)
  std::string ast_visit_implicit_cast(const clang::ImplicitCastExpr * implicit_cast) {
    assert_exception(implicit_cast);
    bool check = implicit_cast->getCastKind() == clang::CastKind::CK_IntegralCast
                 or implicit_cast->getCastKind() == clang::CastKind::CK_IntegralToBoolean;
    if (check == true) {
      throw std::logic_error("Found ImplicitCastExpr with a cast of type " +
                             std::string(implicit_cast->getCastKindName()) +
                             " in expression " +
                             clang_stmt_printer(implicit_cast));
    }
    return AstVisitor::ast_visit_implicit_cast(implicit_cast);
  }
};

#endif  // INT_TYPE_CHECKER_H_
