#ifndef PROG_TRANSFORMS_H_
#define PROG_TRANSFORMS_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Catalogue the core functions corresponding to each program transformation here.

/// Simple strength reduction: rewrite if (1) ? x : y to x.
/// Simplify 1 && x to x");
std::pair<std::string, std::vector<std::string>> strength_reducer(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

// Expr propagation: replace y = b + c; a = y;
// with y=b+c; a=b+c; In some sense we are inverting
// common subexpression elimination
std::pair<std::string, std::vector<std::string>> expr_prop(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

#endif // PROG_TRANSFORMS_H_

