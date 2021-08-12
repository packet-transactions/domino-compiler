#include "const_prop.h"

#include <iostream>
#include <functional>

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "unique_identifiers.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::string const_prop_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, const_prop_body);
}

std::pair<std::string, std::vector<std::string>> const_prop_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused))) {
  std::string transformed_body = "";

  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Check that it's in ssa.
  assert_exception(is_in_ssa(function_body));

  // Now check if any variables in fact equals a constant.
  // We do not need to check for the case where the RHS-expression of a variable assignment
  // is an expression reducible to a constant, since that is already handled by the
  // algebraic simplification pass.
  std::map<std::string, std::string> const_vars;
  for (const auto * child : function_body->children()) {
    // Extract packet variable (i.e. only look at assignment statements.)
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();
    const std::string pkt_var = clang_stmt_printer(lhs);
    if (isa<IntegerLiteral>(rhs)) {
      const_vars[clang_stmt_printer(lhs)] = std::to_string(dyn_cast<IntegerLiteral>(rhs)->getValue().getSExtValue());
    }
  }

  // Now carry out replacements.
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    assert_exception(dyn_cast<BinaryOperator>(child)->isAssignmentOp());
    transformed_body += replace_vars(dyn_cast<BinaryOperator>(child)->getLHS(), const_vars,
                                    {{VariableType::STATE_SCALAR, false}, {VariableType::STATE_ARRAY, false}, {VariableType::PACKET, true}}) +
                        " = " +
                        replace_vars(dyn_cast<BinaryOperator>(child)->getRHS(), const_vars,
                                    {{VariableType::STATE_SCALAR, false}, {VariableType::STATE_ARRAY, false}, {VariableType::PACKET, true}}) +
                        ";";
  }

  return std::make_pair("{" + transformed_body + "}", std::vector<std::string>());
}
