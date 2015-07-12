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
  static std::set<std::string> get_all_vars(const clang::Expr * expr);
};

#endif // EXPR_FUNCTIONS_H_
