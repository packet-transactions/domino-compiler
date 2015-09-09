#ifndef STATEFUL_FLANKS_H_
#define STATEFUL_FLANKS_H_

#include <utility>
#include <string>
#include <vector>
#include <set>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

/// Intermediate representation where we have a read prologue in which
/// all state variables are read into temporary variables. Then the rest
/// of the program operates on these temporary variables. We close the program
/// with a write epilogue that takes temporary variables and writes them into state variables again
std::pair<std::string, std::vector<std::string>> add_stateful_flanks(const clang::CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & id_set);

/// Replace subscript expression in an array with the supplied new_subscript
/// string. Turns a[anything] into a[new_subscript]
std::string replace_subscript_expr(const clang::ArraySubscriptExpr * array_op, const std::string & new_subscript);

// Entry point to code that adds stateful flanks to a function body
std::string stateful_flank_transform(const clang::TranslationUnitDecl * tu_decl);

#endif  // STATEFUL_FLANKS_H_
