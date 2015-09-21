#ifndef CSI_H_
#define CSI_H_

#include <set>
#include <string>
#include <utility>
#include <map>
#include "clang/AST/Expr.h"

/// Map from variable names to their expressions
/// This is preprocessing so that we can compute the ASTs for each variable.
typedef std::map<const std::string, const clang::Expr *> VarMap;

/// Common subexpression identification (CSI) entry point
std::string csi_transform(const clang::TranslationUnitDecl * tu_decl);

/// Main function for CSI that rewrites function body by
/// identifying common sub expressions.
std::pair<std::string, std::vector<std::string>> csi_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

/// Check if two packet variables are equal recursively
/// by comparing their expression trees.
/// Execution starts at check_pkt_var, which calls check_expr, which in turn
/// calls one of check_bin_op or check_un_op,
/// which could then recursively call check_pkt_var till it bottoms out.
bool check_pkt_var(const std::string & pkt1, const std::string & pkt2,
                   const VarMap & var_map);

/// Check if two expressions are equal by delegating
/// to one of check_bin_op or check_un_op.
/// In the future, we can add Conditionals, CallExpr, ArraySubscriptExpr
/// and so on.
bool check_expr(const clang::Expr * expr1, const clang::Expr * expr2,
                const VarMap & var_map);

/// Check if two function call expressions are equal recursively
/// by calling check_pkt_var on corresponding pairs of arguments.
bool check_call_expr(const clang::CallExpr * ce1, const clang::CallExpr * ce2,
                     const VarMap & var_map);

/// Check if two unary operators are equal recursively
/// by calling check_pkt_var on the sub expression of the unary operator
bool check_un_op(const clang::UnaryOperator * un1, const clang::UnaryOperator * un2,
                 const VarMap & var_map);

/// Check if two binary ops are equal recursively
/// by calling check_pkt_var on each of the operands.
bool check_bin_op(const clang::BinaryOperator * bin1, const clang::BinaryOperator * bin2,
                  const VarMap & var_map);

#endif // CSI_H_
