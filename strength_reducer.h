#ifndef STRENGTH_REDUCER_H_
#define STRENGTH_REDUCER_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Entry point from SinglePass,
/// which immediately delegates to strength_reduce_body
std::string strength_reducer_transform(const clang::TranslationUnitDecl * tu_decl);

/// Simple strength reduction: rewrite if (1) ? x : y to x.
/// Simplify 1 && x to x");
std::pair<std::string, std::vector<std::string>> strength_reduce_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

#endif // STRENGTH_REDUCER_H_
