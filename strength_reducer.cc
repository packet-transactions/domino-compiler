#include <iostream>
#include <string>
#include "clang/AST/AST.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang_utility_functions.h"
#include "single_pass.h"

using namespace clang;
using namespace clang::tooling;

/// The actual strength reduction
static std::string strength_reduce_helper(const FunctionDecl* function_decl) {
  assert(function_decl);

  // Rewrite function body
  std::string transformed_body = "";
  const auto * function_body = function_decl->getBody();
  assert(isa<CompoundStmt>(function_body));
  for (const auto & child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts for RHS
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();

    if(not isa<ConditionalOperator>(rhs)) {
      // If it isn't a conditional operator, pass it through
      transformed_body += clang_stmt_printer(bin_op) + ";";
    } else {
      const auto * cond_op = dyn_cast<ConditionalOperator>(rhs);

      // Get condition expression
      const auto * cond = cond_op->getCond()->IgnoreParenImpCasts();

      if(isa<IntegerLiteral>(cond)) {
        // TODO: Check that it's the integer literal one
        // replace with straight-up assignment
        transformed_body += clang_stmt_printer(bin_op->getLHS()) + " = " + clang_stmt_printer(cond_op->getTrueExpr()) + ";"; 
      } else if (isa<BinaryOperator>(cond)) {
        // Has to be an &&, otherwise assert
        const auto * cond_predicate = dyn_cast<BinaryOperator>(cond);
        assert(BinaryOperator::getOpcodeStr(cond_predicate->getOpcode()) == "&&");

        // Assuming it's &&, get left and right components
        const auto * left_bool_op = cond_predicate->getLHS();
        const auto * right_bool_op = cond_predicate->getRHS();

        // If both left_bool_op and right_bool_op are IntegerLiteral, something's wrong
        assert (not (isa<IntegerLiteral>(left_bool_op) and isa<IntegerLiteral>(right_bool_op)));

        // If either of the two is an IntegerLiteral, return the other
        // TODO: Check that it's actually 1.
        const Expr * simplified_pred = nullptr;
        if (isa<IntegerLiteral>(left_bool_op)) {
          simplified_pred = right_bool_op;
        } else if (isa<IntegerLiteral>(right_bool_op)) {
          simplified_pred = left_bool_op;
        } else {
          simplified_pred = cond_predicate;
        }

        transformed_body += clang_stmt_printer(bin_op->getLHS()) + " = " + clang_stmt_printer(simplified_pred)
                                                       + " ? " + clang_stmt_printer(cond_op->getTrueExpr())
                                                       + " : " + clang_stmt_printer(cond_op->getFalseExpr()) + ";";
      } else if (isa<DeclRefExpr>(cond) or isa<MemberExpr>(cond)) {
        // pass it through, nothing to simplify here
        transformed_body += clang_stmt_printer(bin_op) + ";";
      } else {
        assert(false);
      }
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

/// Strength Reduction transformation
static std::string strength_reducer_transform(const TranslationUnitDecl * tu_decl) {
  std::string ret;
  // Loop through all declarations within the translation unit decl
  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl) or
        isa<RecordDecl>(child_decl) or
        (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl))))) {
      ret += clang_decl_printer(child_decl) + ";";
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      ret += strength_reduce_helper(dyn_cast<FunctionDecl>(child_decl));
    }
  }
  return ret;
}



static llvm::cl::OptionCategory strength_redux(""
"Simple strength reduction: rewrite if (1) ? x : y to x."
"Simplify 1 && x to x");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, strength_redux);

  // Parse file once and output it
  std::cout << SinglePass(op, strength_reducer_transform).output();

  return 0;
}
