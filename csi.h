#ifndef CSI_H_
#define CSI_H_

#include <set>
#include <string>
#include <utility>
#include <map>
#include "clang/AST/Expr.h"

// Map from variable names to their expressions
typedef std::map<const std::string, const clang::Expr *> VarMap;

// Common subexpression identification (CSI) entry point
std::string csi_transform(const clang::TranslationUnitDecl * tu_decl);

// Main function for CSI that rewrites function body
std::pair<std::string, std::vector<std::string>> csi_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

// Check if two packet variables are equal
bool check_pkt_var(const std::string & pkt1, const std::string & pkt2,
                   const VarMap & var_map);

// Check if two unary operators are equal
bool check_un_op(const clang::UnaryOperator * un1, const clang::UnaryOperator * un2,
                 const VarMap & var_map);

// Check if two binary ops are equal
bool check_bin_op(const clang::BinaryOperator * bin1, const clang::BinaryOperator * bin2,
                  const VarMap & var_map);

// Check if two expressions are equal
bool check_expr(const clang::Expr * expr1, const clang::Expr * expr2,
                const VarMap & var_map);

#endif // CSI_H_
