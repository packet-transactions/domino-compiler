#include <iostream>
#include <set>
#include <string>
#include <utility>

#include "util.h"
#include "clang_utility_functions.h"
#include "packet_variable_census.h"
#include "pkt_func_transform.h"
#include "unique_var_generator.h"
#include "single_pass.h"
#include "expr_functions.h"

using namespace clang;

static std::pair<std::string, std::vector<std::string>> stateful_flank_transform(const CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & packet_var_set) {
  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Create unique variable generator
  UniqueVarGenerator unique_var_gen(packet_var_set);

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
        const auto new_tmp_var = unique_var_gen.get_unique_var(state_var);
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

static std::string help_string(""
"Intermediate representation where we have a read prologue in which"
"all state variables are read into temporary variables. Then the rest"
"of the program operates on these temporary variables. We close the program"
"with a write epilogue that takes temporary variables and writes them into state"
"variables again");

int main(int argc, const char **argv) {
  // Parse file once and generate set of all packet variables
  const auto packet_var_set = SinglePass<std::set<std::string>>(get_file_name(argc, argv, help_string), packet_variable_census).output();

  // Parse file once and output stateful flanks (read prologues, write epilogues)
  const FuncBodyTransform stateful_flank_converter = std::bind(stateful_flank_transform, std::placeholders::_1, std::placeholders::_2, packet_var_set);

  std::cout << SinglePass<std::string>(get_file_name(argc, argv, help_string), std::bind(pkt_func_transform, std::placeholders::_1, stateful_flank_converter)).output();
}
