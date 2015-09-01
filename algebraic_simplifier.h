#ifndef ALGEBRAIC_SIMPLIFIER_H_
#define ALGEBRAIC_SIMPLIFIER_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Entry point from SinglePass,
/// which immediately delegates to algebraic_simplify_helper
std::string algebraic_simplifier_transform(const clang::TranslationUnitDecl * tu_decl);

// helper function to initiate recursive call of algebraic_simplify_stmt
// on function body
std::pair<std::string, std::vector<std::string>> algebraic_simplify_helper(const clang::CompoundStmt * body,
                                                                           const std::string & pkt_name __attribute__((unused)));

// Recurse on stmt and continue simplifying using algebraic rewrite rules
std::string algebraic_simplify_stmt(const clang::Stmt * stmt);

#endif // ALGEBRAIC_SIMPLIFIER_H_
