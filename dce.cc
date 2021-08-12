#include "dce.h"

#include <functional>
#include <iostream>

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "unique_identifiers.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::string dce_transform(const TranslationUnitDecl *tu_decl) {
  return pkt_func_transform(tu_decl, dce_body);
}

std::pair<std::string, std::vector<std::string>>
dce_body(const clang::CompoundStmt *function_body,
         const std::string &pkt_name __attribute__((unused))) {
  std::string transformed_body = "";

  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Check that it's in ssa.
  assert_exception(is_in_ssa(function_body));
  
  // Since we're after SSA, it suffices
  // to check whether there are any LHSes that
  // never gets used on a RHS throughout the function body.
  std::set<const std::string> lhsVars;
  std::set<const std::string> rhsVars;

  for (const auto *child : function_body->children()) {
    // Extract packet variable (i.e. only look at assignment statements.)
    assert_exception(isa<BinaryOperator>(child));
    const auto *bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto *lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto *rhs = bin_op->getRHS()->IgnoreParenImpCasts();

    // add LHS defines
    lhsVars.insert(clang_stmt_printer(lhs));

    // process RHS variables census.
    const std::set<std::string> &stmtRhsVars =
        gen_var_list(rhs, {{VariableType::PACKET, true},
                           {VariableType::STATE_ARRAY, true},
                           {VariableType::STATE_SCALAR, true},
                           {VariableType::FUNCTION_PARAMETER, true}});
    for (const auto &rhsVar : stmtRhsVars) {
      rhsVars.insert(rhsVar);
    }
  }

  // emptyDefines are lhsVars that _do not_ appear as rhsVars
  // they are exactly the set of dead codes we want to eliminate
  std::set<const std::string> emptyDefines;
  for (const auto &lhsVar : lhsVars)
    if (rhsVars.find(lhsVar) == rhsVars.end())
      emptyDefines.insert(lhsVar);

  // std::cout << "dce_body: found empty defines" << std::endl;
  // Now carry out replacements.
  for (const auto *child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    assert_exception(dyn_cast<BinaryOperator>(child)->isAssignmentOp());
    const auto *bin_op = dyn_cast<BinaryOperator>(child);
    const auto *lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const std::string lhsStr = clang_stmt_printer(lhs);
    // If current assignment stmt is not dead code, then
    // add it to the transformed body. Else, ignore it.
    if (emptyDefines.find(lhsStr) == emptyDefines.end()) {
      transformed_body += clang_stmt_printer(child) + ";";
    }
  }
  return std::make_pair("{" + transformed_body + "}",
                        std::vector<std::string>());
}
