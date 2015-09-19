#include "csi.h"

#include <iostream>
#include <functional>

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "unique_identifiers.h"

using namespace clang;
using std::placeholders::_1;
using std::placeholders::_2;

std::string csi_transform(const TranslationUnitDecl * tu_decl) {
  return pkt_func_transform(tu_decl, csi_body);
}

std::pair<std::string, std::vector<std::string>> csi_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused))) {
  std::string transformed_body = "";

  // Vector of newly created packet temporaries
  std::vector<std::string> new_decls = {};

  // Check that it's in ssa.
  assert_exception(is_in_ssa(function_body));

  // Populate variable to expression map for all packet variables
  VarMap var_map;
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    // Consider only packet variables.
    if (isa<MemberExpr>(lhs)) {
      assert_exception(var_map.find(clang_stmt_printer(dyn_cast<MemberExpr>(lhs))) == var_map.end());
      var_map[clang_stmt_printer(dyn_cast<MemberExpr>(lhs))] = bin_op->getRHS()->IgnoreParenImpCasts();
    }
  }

  // Now check for equality among all MemberExprs.
  // Scan MemberExprs in lexical order so that we can identify the last of them.
  // Add equal MemberExprs to a vector of vectors to create equivalence partitions.
  std::vector<std::vector<std::string>> partitions;
  for (const auto * child : function_body->children()) {
    // Extract packet variable
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const std::string pkt_var = clang_stmt_printer(lhs);

    // Check if this packet variable to equal to any packet variable
    // in any equivalence partition. If so, add it to that partition
    // If no such partition exists, create a new partition.
    bool found_partition = false;
    for (auto & partition : partitions) {
      if (std::find_if(partition.begin(), partition.end(),
                       [pkt_var, & var_map] (const auto & var) { return check_pkt_var(var, pkt_var, var_map); })
          != partition.end()) {
        partition.emplace_back(pkt_var);
        found_partition = true;
        break;
      }
    }

    // Create a new partition for this variable alone
    if (found_partition == false) {
      partitions.emplace_back(std::vector<std::string>({pkt_var}));
    }
  }

  // Now use one exemplar variable for each partition, by creating a replacement map for the rest.
  // We pick the last one in lexical order to preserve SSAs renames
  std::map<std::string, std::string> repl_map;
  for (const auto & partition : partitions) {
    if (partition.size() > 1) {
      // Set it up so that all variables in the equivalence class
      // are renamed to the last variable (this preserves the SSA renames for jayhawk).
      std::string repl_candidate = partition.back();
      for (const auto & pkt_var : partition) {
        repl_map[pkt_var] = repl_candidate;
      }
    }
  }

  // Now carry out replacements.
  for (const auto * child : function_body->children()) {
    assert_exception(isa<BinaryOperator>(child));
    assert_exception(dyn_cast<BinaryOperator>(child)->isAssignmentOp());
    transformed_body += replace_vars(dyn_cast<BinaryOperator>(child)->getLHS(), repl_map,
                                    {{VariableType::STATE_SCALAR, false}, {VariableType::STATE_ARRAY, false}, {VariableType::PACKET, true}}) +
                        " = " +
                        replace_vars(dyn_cast<BinaryOperator>(child)->getRHS(), repl_map,
                                    {{VariableType::STATE_SCALAR, false}, {VariableType::STATE_ARRAY, false}, {VariableType::PACKET, true}}) +
                        ";";
  }

  return std::make_pair("{" + transformed_body + "}", std::vector<std::string>());
}

bool check_pkt_var(const std::string & pkt1, const std::string & pkt2,
                   const VarMap & var_map) {
  if (pkt1 == pkt2) {
    // Base case, string comparison
    return true;
  } else if ((var_map.find(pkt1) != var_map.end()) and (var_map.find(pkt2) != var_map.end())) {
    // Recursion (strictly this is mutual recursion because check_expr calls check_bin_op or check_un_op,
    // which then calls check_pkt_var.
    return check_expr(var_map.at(pkt1), var_map.at(pkt2), var_map);
  } else {
    // If either doesn't exist in var maps (it's an input variable), just return false.
    return false;
  }
}

bool check_expr(const Expr * expr1, const Expr * expr2,
                const VarMap & var_map) {
  if (isa<BinaryOperator>(expr1) and isa<BinaryOperator>(expr2)) {
    return check_bin_op(dyn_cast<BinaryOperator>(expr1), dyn_cast<BinaryOperator>(expr2), var_map);
  } else if (isa<IntegerLiteral>(expr1) and isa<IntegerLiteral>(expr2)) {
    // TODO: This string compare is a hack, but should work.
    return clang_stmt_printer(expr1) == clang_stmt_printer(expr2);
  } else if (isa<UnaryOperator>(expr1) and isa<UnaryOperator>(expr2)) {
    return check_un_op(dyn_cast<UnaryOperator>(expr1), dyn_cast<UnaryOperator>(expr2), var_map);
  } else {
    // TODO: We don't check conditional ops or CallExprs for equality right now.
    // This is correct, although pessimal.
    return false;
  }
}

bool check_un_op(const UnaryOperator * un1, const UnaryOperator * un2,
                 const VarMap & var_map) {
  // If opcode isn't equal return right away
  if (un1->getOpcode() != un2->getOpcode()) return false;

  // Extract arguments
  const auto * un1_sub_expr = un1->getSubExpr()->IgnoreParenImpCasts();
  const auto * un2_sub_expr = un2->getSubExpr()->IgnoreParenImpCasts();
  assert_exception(isa<MemberExpr>(un1_sub_expr) or isa<IntegerLiteral>(un1_sub_expr));
  assert_exception(isa<MemberExpr>(un2_sub_expr) or isa<IntegerLiteral>(un2_sub_expr));

  // Recursively check for equality
  if (isa<MemberExpr>(un1_sub_expr) and isa<MemberExpr>(un2_sub_expr)) return check_pkt_var(clang_stmt_printer(un1_sub_expr), clang_stmt_printer(un2_sub_expr), var_map);
  else if (isa<IntegerLiteral>(un1_sub_expr) and isa<IntegerLiteral>(un2_sub_expr)) return clang_stmt_printer(un1_sub_expr) == clang_stmt_printer(un2_sub_expr);
  else return false;
}

bool check_bin_op(const BinaryOperator * bin1, const BinaryOperator * bin2,
                  const VarMap & var_map) {
  // If opcodes don't match up, return right away.
  if (bin1->getOpcode() != bin2->getOpcode()) return false;

  // Extract arguments
  const auto * lhs1 = bin1->getLHS()->IgnoreParenImpCasts();
  const auto * lhs2 = bin2->getLHS()->IgnoreParenImpCasts();
  assert_exception(isa<MemberExpr>(lhs1) or isa<IntegerLiteral>(lhs1) or isa<CallExpr>(lhs1) or isa<ArraySubscriptExpr>(lhs1));
  assert_exception(isa<MemberExpr>(lhs2) or isa<IntegerLiteral>(lhs2) or isa<CallExpr>(lhs2) or isa<ArraySubscriptExpr>(lhs2));

  const auto * rhs1 = bin1->getRHS()->IgnoreParenImpCasts();
  const auto * rhs2 = bin2->getRHS()->IgnoreParenImpCasts();
  assert_exception(isa<MemberExpr>(rhs1) or isa<IntegerLiteral>(rhs1) or isa<CallExpr>(rhs1) or isa<ArraySubscriptExpr>(rhs1));
  assert_exception(isa<MemberExpr>(rhs2) or isa<IntegerLiteral>(rhs2) or isa<CallExpr>(rhs2) or isa<ArraySubscriptExpr>(rhs2));

  if (isa<MemberExpr>(lhs1) and isa<MemberExpr>(lhs2) and isa<MemberExpr>(rhs1) and isa<MemberExpr>(rhs2)) {
    return check_pkt_var(clang_stmt_printer(dyn_cast<MemberExpr>(lhs1)), clang_stmt_printer(dyn_cast<MemberExpr>(lhs2)), var_map) and
           check_pkt_var(clang_stmt_printer(dyn_cast<MemberExpr>(rhs1)), clang_stmt_printer(dyn_cast<MemberExpr>(rhs2)), var_map);
  } else if (isa<MemberExpr>(lhs1) and isa<MemberExpr>(lhs2) and isa<IntegerLiteral>(rhs1) and isa<IntegerLiteral>(rhs2)) {
    return check_pkt_var(clang_stmt_printer(dyn_cast<MemberExpr>(lhs1)), clang_stmt_printer(dyn_cast<MemberExpr>(lhs2)), var_map) and
           (clang_stmt_printer(rhs1) == clang_stmt_printer(rhs2));
  } else if (isa<IntegerLiteral>(lhs1) and isa<IntegerLiteral>(lhs2) and isa<MemberExpr>(rhs1) and isa<MemberExpr>(rhs2)) {
    return (clang_stmt_printer(lhs1) == clang_stmt_printer(lhs2)) and
           check_pkt_var(clang_stmt_printer(dyn_cast<MemberExpr>(rhs1)), clang_stmt_printer(dyn_cast<MemberExpr>(rhs2)), var_map);
  } else {
    return false;
  }
}
