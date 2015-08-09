#include "prog_transforms.h"

#include <set>
#include <string>
#include <iostream>

#include "clang_utility_functions.h"
#include "expr_functions.h"
#include "unique_var_generator.h"

using namespace clang;

std::pair<std::string, std::vector<std::string>> ssa_transform(const CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & packet_var_set) {
  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Create unique variable generator
  UniqueVarGenerator unique_var_gen(packet_var_set);

  // All indices where every packet variable is defined.
  // We choosen to rename ALL definitions of a packet variable
  // rather than just the redefinitions because there might
  // be reads preceding the first definition and it's correct to rename all definitions.
  std::map<std::string, std::vector<int>> def_locs;
  int index = 0;
  for (const auto * child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();

    if (isa<MemberExpr>(lhs)) {
      if (def_locs.find(clang_stmt_printer(lhs)) == def_locs.end()) {
        def_locs[clang_stmt_printer(lhs)] = {index};
      } else {
        def_locs.at(clang_stmt_printer(lhs)).emplace_back(index);
      }
    }
    index++;
  }

  // Now, do the renaming, storing output in function_body
  std::string function_body_str;
  index = 0;
  std::map<std::string, std::string> replacements;
  for (const auto * child : function_body->children()) {
    assert(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert(bin_op->isAssignmentOp());

    // First rewrite RHS using whatever replacements we currently have
    const std::string rhs_str = ExprFunctions::replace_vars(bin_op->getRHS(), replacements);

    // Now look at LHS
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    if (isa<MemberExpr>(lhs)) {
      // Is this a redefinition?
      const std::string lhs_var = clang_stmt_printer(lhs);
      assert(def_locs.find(lhs_var) != def_locs.end());
      const bool is_redef = std::find(def_locs.at(lhs_var).begin(), def_locs.at(lhs_var).end(), index) != def_locs.at(lhs_var).end();

      // If so, modify replacements
      if (is_redef) {
        const auto var_type    = dyn_cast<MemberExpr>(lhs)->getMemberDecl()->getType().getAsString();
        const auto new_tmp_var = unique_var_gen.get_unique_var(dyn_cast<MemberExpr>(lhs)->getMemberDecl()->getNameAsString());
        const auto var_decl    = var_type + " " + new_tmp_var + ";";
        new_decls.emplace_back(var_decl);
        replacements[lhs_var] =  pkt_name + "." + new_tmp_var;
      }
    }

    // Now rewrite LHS
    assert(bin_op->getLHS());
    const std::string lhs_str = ExprFunctions::replace_vars(bin_op->getLHS(), replacements);
    function_body_str += lhs_str + " = " + rhs_str + ";";
    index++;
  }

  return std::make_pair(function_body_str, new_decls);
}
