#ifndef PROG_TRANSFORMATIONS_H_
#define PROG_TRANSFORMATIONS_H_

#include <vector>
#include <string>
#include "clang/AST/Stmt.h"

/// Catalogue the core functions corresponding to each program transformation here.

/// Simple strength reduction: rewrite if (1) ? x : y to x.
/// Simplify 1 && x to x");
std::pair<std::string, std::vector<std::string>> strength_reducer(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

#endif  // PROG_TRANSFORMATIONS_H_
