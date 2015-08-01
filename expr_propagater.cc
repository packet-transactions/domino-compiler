#include <iostream>
#include <string>
#include "clang/AST/AST.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang_utility_functions.h"
#include "single_pass.h"

using namespace clang;
using namespace clang::tooling;

/// The actual expression propagation
static std::string expr_prop_helper(const FunctionDecl* function_decl) {
  assert(function_decl);

  // Maintain map from variable name (packet or state variable)
  // to a string representing its expression, for expression propagation.
  std::map<std::string, std::string> var_to_expr;

  // Rewrite function body
  std::string transformed_body = "";
  const auto * function_body = function_decl->getBody();
  assert(isa<CompoundStmt>(function_body));
  for (const auto & child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts for LHS and RHS
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    // Populate var_to_expr
    var_to_expr[clang_stmt_printer(lhs)] = clang_stmt_printer(rhs);

    if ((isa<DeclRefExpr>(rhs) or isa<MemberExpr>(rhs)) and (var_to_expr.find(clang_stmt_printer(rhs)) != var_to_expr.end())) {
      // If rhs is a packet/state variable, replace it with its current expr
      transformed_body += clang_stmt_printer(lhs) + "=" + var_to_expr.at(clang_stmt_printer(rhs)) + ";";
    } else {
      // Pass through
      transformed_body += clang_stmt_printer(lhs) + "=" + clang_stmt_printer(rhs) + ";";
    }
  }

  // Append function body to signature
  assert(function_decl->getNumParams() >= 1);
  const auto * pkt_param = function_decl->getParamDecl(0);
  const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
  const auto pkt_name = clang_value_decl_printer(pkt_param);

  // Get transformed_body string
  return function_decl->getReturnType().getAsString() + " " +
         function_decl->getNameInfo().getName().getAsString() +
         "( " + pkt_type + " " +  pkt_name + ") { " +
         transformed_body + "}\n";
}

/// Expression propagation transformation
static std::string expr_prop_transform(const TranslationUnitDecl * tu_decl) {
  std::string ret;
  // Loop through all declarations within the translation unit decl
  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl) or
        isa<RecordDecl>(child_decl) or
        (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl))))) {
      ret += clang_decl_printer(child_decl) + ";";
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      ret += expr_prop_helper(dyn_cast<FunctionDecl>(child_decl));
    }
  }
  return ret;
}



static llvm::cl::OptionCategory expr_prop(""
"Expr propagation: replace y = b + c; a = y;"
"with y=b+c; a=b+c; In some sense we are inverting"
"the process of common subexpression elimination");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, expr_prop);

  // Parse file once and output it
  std::cout << SinglePass(op, expr_prop_transform).output();

  return 0;
}
