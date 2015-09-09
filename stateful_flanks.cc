#include "stateful_flanks.h"

#include <iostream>
#include <functional>

#include "third_party/assert_exception.h"

#include "clang/AST/Expr.h"

#include "clang_utility_functions.h"
#include "unique_identifiers.h"
#include "pkt_func_transform.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::pair<std::string, std::vector<std::string>> add_stateful_flanks(const CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & id_set) {
  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Generate unique identifiers for packet temporaries,
  // when replacing state variables with packet variables
  UniqueIdentifiers unique_identifiers(id_set);

  // Run through function_body, storing all lhs->rhs mappings
  // i.e. variable (packet, state scalar, or state array) to expression
  typedef std::string VariableName;
  typedef std::string Expression;
  std::map<VariableName, Expression> var_expr_map;
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto * rhs = bin_op->getRHS()->IgnoreParenImpCasts();
    var_expr_map[clang_stmt_printer(lhs)] = clang_stmt_printer(rhs);
  }

  // 1. Identify all stateful variables in the program.
  // 2. Create a read prologue for all of them: each state variable is read into a packet temporary.
  // 3. Create a write epilogue for all of them: the packet temporary from 2. is written back into state.
  // 4. Populate a state variable table to replace state variables with temporaries in the function body.
  std::string read_prologue = "";
  std::string write_epilogue = "";
  std::map<std::string, std::string> state_var_table;
  std::set<std::string> subscript_vars;
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts on lhs
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    // If lhs is a DeclRefExpr or an ArraySubscriptExpr, it's a stateful variable
    // It's sufficient to inspect LHS alone to determine stateful variables,
    // because we are assuming a stateful variable is written at least once.
    // Otherwise, I don't see the point of a stateful variable.
    if (isa<DeclRefExpr>(lhs) or isa<ArraySubscriptExpr>(lhs)) {
      const std::string  state_var = clang_stmt_printer(lhs);
      if (state_var_table.find(state_var) == state_var_table.end()) {
        const auto var_type    = isa<DeclRefExpr>(lhs) ? dyn_cast<DeclRefExpr>(lhs)->getDecl()->getType().getAsString() :
                                                         dyn_cast<ArraySubscriptExpr>(lhs)->getType().getAsString();
        const auto new_tmp_var = unique_identifiers.get_unique_identifier(isa<DeclRefExpr>(lhs) ? state_var
                                                                                                : clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(lhs)->getBase()));
        const auto var_decl    = var_type + " " + new_tmp_var + ";";
        new_decls.emplace_back(var_decl);

        const auto pkt_tmp_var   = pkt_name + "." + new_tmp_var;

        // Read from a state variable into a packet temporary
        // If it's an array, read index into a newly created packet temporary and move to prologue too
        if (isa<ArraySubscriptExpr>(lhs)) {
          assert_exception(isa<MemberExpr>(dyn_cast<ArraySubscriptExpr>(lhs)->getIdx()->IgnoreParenImpCasts()));
          const auto subscript_var = clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(lhs)->getIdx());
          subscript_vars.emplace(subscript_var);
          const auto pkt_field_in_subscript = dyn_cast<MemberExpr>(dyn_cast<ArraySubscriptExpr>(lhs)->getIdx()->IgnoreParenImpCasts())->getMemberDecl()->getNameAsString();

          //  Create a temporary variable for subscript_var
          const auto new_tmp_var_for_subscript = unique_identifiers.get_unique_identifier(pkt_field_in_subscript);
          const auto subscript_var_decl        = "int " + new_tmp_var_for_subscript + ";";
          new_decls.emplace_back(subscript_var_decl);

          // Prefix it with "packet."
          const auto new_subscript_var   = pkt_name + "." + new_tmp_var_for_subscript;

          // Create read and write flanks for it
          read_prologue += new_subscript_var + " = " + var_expr_map.at(subscript_var) + ";";
          read_prologue += pkt_tmp_var + " = " + replace_subscript_expr(dyn_cast<ArraySubscriptExpr>(lhs), new_subscript_var) + ";";
          write_epilogue += replace_subscript_expr(dyn_cast<ArraySubscriptExpr>(lhs), new_subscript_var) + " = " + pkt_tmp_var + ";";

          // Print out the rename of subscript_var for jayhawk to use
          std::cerr << "// " << pkt_field_in_subscript << " " << new_tmp_var_for_subscript << std::endl;
        } else {
          read_prologue += pkt_tmp_var + " = " + state_var + ";";
          write_epilogue += state_var + " = " + pkt_tmp_var + ";";
        }
        state_var_table[state_var] = pkt_tmp_var;
      }
    }
  }

  // Now, replace all occurences of the stateful variables throughout the code
  std::string function_body_str;
  for (const auto * child : function_body->children()) {
     assert_exception(isa<BinaryOperator>(child));
     const auto * bin_op = dyn_cast<BinaryOperator>(child);
     assert_exception(bin_op->isAssignmentOp());

     if (subscript_vars.find(clang_stmt_printer(bin_op->getLHS())) != subscript_vars.end()) {
       // This is a subscript variable, it's already been moved into the prologue
       continue;
     }

     function_body_str +=   replace_vars(bin_op->getLHS(), state_var_table, {{VariableType::STATE_SCALAR, true}, {VariableType::STATE_ARRAY, true}, {VariableType::PACKET, false}}) + " = "
                          + replace_vars(bin_op->getRHS(), state_var_table, {{VariableType::STATE_SCALAR, true}, {VariableType::STATE_ARRAY, true}, {VariableType::PACKET, false}}) + ";";
  }

  return std::make_pair("{" + read_prologue + "\n\n" +  function_body_str + "\n\n" + write_epilogue + "}", new_decls);
}

std::string replace_subscript_expr(const ArraySubscriptExpr * array_op, const std::string & new_subscript) {
  assert_exception(array_op);
  assert_exception(isa<MemberExpr>(array_op->getIdx()->IgnoreParenImpCasts()));
  return clang_stmt_printer(array_op->getBase()) + "[" + new_subscript + "]";
}

std::string stateful_flank_transform(const TranslationUnitDecl * tu_decl) {
  const auto & id_set = identifier_census(tu_decl);
  return pkt_func_transform(tu_decl, std::bind(add_stateful_flanks, _1, _2, id_set));
}
