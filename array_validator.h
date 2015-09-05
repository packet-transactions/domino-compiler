#ifndef ARRAY_VALIDATOR_H_
#define ARRAY_VALIDATOR_H_

#include <string>
#include <utility>

#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Validate array accesses in domino
std::string array_validator_transform(const clang::TranslationUnitDecl * tu_decl);

/// Execute echo on function body after
/// checking that there are no incorrect array acceses
/// Specifically, check that the packet field used to index an array isn't modified
/// after being used as an array subscript
/// (it's constant over the duration of a packet).
std::pair<std::string, std::vector<std::string>> array_validate_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__ ((unused)));

/// Recursively check that an array expression for a particular array
/// is indexed only by a MemberRefExpr (a packet field).
/// For all other expressions, return true.
bool check_array_subscript_expr(const clang::Stmt * stmt);

#endif  // ARRAY_VALIDATOR_H_
