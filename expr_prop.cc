#include "expr_prop.h"

#include "third_party/assert_exception.h"

#include "pkt_func_transform.h"
#include "clang_utility_functions.h"

using namespace clang;

std::string expr_prop_transform(const clang::TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, expr_prop_fn_body);
}

std::pair<std::string, std::vector<std::string>> expr_prop_fn_body(const CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused))) {
  // Maintain map from variable name (packet or state variable)
  // to a string representing its expression, for expression propagation.
  std::map<std::string, std::string> var_to_expr;

  // Rewrite function body
  std::string transformed_body = "";
  assert_exception(function_body);
  for (const auto & child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts for LHS and RHS
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    // Populate var_to_expr, so long as the lhs variable doesn't
    // appear within the rhs expression
    if (clang_stmt_printer(rhs).find(clang_stmt_printer(lhs)) == std::string::npos) {
      var_to_expr[clang_stmt_printer(lhs)] = clang_stmt_printer(rhs);
    }

    if ((isa<DeclRefExpr>(rhs) or isa<MemberExpr>(rhs)) and (var_to_expr.find(clang_stmt_printer(rhs)) != var_to_expr.end())) {
      // If rhs is a packet/state variable, replace it with its current expr
      transformed_body += clang_stmt_printer(lhs) + "=" + var_to_expr.at(clang_stmt_printer(rhs)) + ";";
    } else {
      // Pass through
      transformed_body += clang_stmt_printer(lhs) + "=" + clang_stmt_printer(rhs) + ";";
    }
  }
  return std::make_pair(transformed_body, std::vector<std::string>());
}
