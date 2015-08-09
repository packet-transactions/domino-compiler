#ifndef PROG_TRANSFORMATIONS_H_
#define PROG_TRANSFORMATIONS_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"

/// Catalogue the core functions corresponding to each program transformation here.

/// Simple strength reduction: rewrite if (1) ? x : y to x.
/// Simplify 1 && x to x");
std::pair<std::string, std::vector<std::string>> strength_reducer(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

// Expr propagation: replace y = b + c; a = y;
// with y=b+c; a=b+c; In some sense we are inverting
// common subexpression elimination
std::pair<std::string, std::vector<std::string>> expr_prop(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

/// Intermediate representation where we have a read prologue in which
/// all state variables are read into temporary variables. Then the rest
/// of the program operates on these temporary variables. We close the program
/// with a write epilogue that takes temporary variables and writes them into state
/// variables again
std::pair<std::string, std::vector<std::string>> stateful_flank_transform(const clang::CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & packet_var_set);

#endif  // PROG_TRANSFORMATIONS_H_
