#ifndef EXPR_FUNCTIONS_H_
#define EXPR_FUNCTIONS_H_

#include <string>
#include <set>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"

/// Utility functions that operate on clang::Expr ASTs
/// For now, it analyzes a clang::Expr to determine the
/// set of variables appearing within it.
/// Later, we could probably use it for expression flattening
class ExprFunctions {
 public:
  /// Get all variables (state and packet) referenced within expr
  static std::set<std::string> get_vars(const clang::Expr * expr);

  /// Replace a specific DeclRefExpr* with a new string within expr
  static std::string replace_vars(const clang::Expr * expr, const std::map<std::string, std::string> & repl_map);

};

#endif // EXPR_FUNCTIONS_H_
