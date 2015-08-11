#include "stateful_flanks.h"

#include <functional>

#include "clang_utility_functions.h"
#include "unique_identifiers.h"
#include "expr_functions.h"
#include "identifier_census.h"
#include "pkt_func_transform.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::pair<std::string, std::vector<std::string>> add_stateful_flanks(const CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & id_set) {
  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Create unique identifiers object
  UniqueIdentifiers unique_identifiers(id_set);

  // 1. Identify all stateful variables in the program.
  // 2. Create a read prologue for all of them: each state variable is read into a packet temporary.
  // 3. Create a write epilogue for all of them: the packet temporary from 2. is written back into state.
  // 4. Populate a state variable table to replace state variables with temporaries in the function body.
  std::string read_prologue = "";
  std::string write_epilogue = "";
  std::map<std::string, std::string> state_var_table;
  for (const auto * child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    // Strip off parenthesis and casts on lhs
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    // If lhs is a DeclRefExpr, it's a stateful variable
    if (isa<DeclRefExpr>(lhs)) {
      const std::string  state_var = clang_stmt_printer(lhs);
      if (state_var_table.find(state_var) == state_var_table.end()) {
        const auto var_type    = dyn_cast<DeclRefExpr>(lhs)->getDecl()->getType().getAsString();
        const auto new_tmp_var = unique_identifiers.get_unique_identifier(state_var);
        const auto var_decl    = var_type + " " + new_tmp_var + ";";
        new_decls.emplace_back(var_decl);

        const auto pkt_tmp_var   = pkt_name + "." + new_tmp_var;
        read_prologue += pkt_tmp_var + " = " + state_var + ";";
        write_epilogue += state_var + " = " + pkt_tmp_var + ";";
        state_var_table[state_var] = pkt_tmp_var;
      }
    }
  }

  // Now, replace all occurences of the stateful variables throughout the code
  std::string function_body_str;
  for (const auto * child : function_body->children()) {
     assert(isa<BinaryOperator>(child));
     const auto * bin_op = dyn_cast<BinaryOperator>(child);
     assert(bin_op->isAssignmentOp());

     function_body_str +=   ExprFunctions::replace_vars(bin_op->getLHS(), state_var_table) + " = "
                          + ExprFunctions::replace_vars(bin_op->getRHS(), state_var_table) + ";";
  }

  return std::make_pair(read_prologue + "\n\n" +  function_body_str + "\n\n" + write_epilogue, new_decls);
}

std::string stateful_flank_transform(const TranslationUnitDecl * tu_decl) {
  const auto & id_set = identifier_census(tu_decl);
  return pkt_func_transform(tu_decl, std::bind(add_stateful_flanks, _1, _2, id_set));
}
