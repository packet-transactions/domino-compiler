#ifndef INT_TYPE_CHECKER_H_
#define INT_TYPE_CHECKER_H_

#include <string>
#include <utility>

#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Simple type checker for integers.
std::string int_type_checker_transform(const clang::TranslationUnitDecl * tu_decl);

/// Execute echo on function body after
/// checking that there are no type errors
std::pair<std::string, std::vector<std::string>> int_type_check_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

/// Checks that there are no ImplicitCastExprs that invoke an IntegralCast
/// (in some ways, a poor man's version of Wconversion and Wsign-conversion)
bool check_for_implicit_casts(const clang::Stmt* stmt);

#endif  // INT_TYPE_CHECKER_H_
