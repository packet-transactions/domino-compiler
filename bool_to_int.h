#ifndef BOOL_TO_INT_H_
#define BOOL_TO_INT_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Entry point from SinglePass,
/// which immediately delegates to bool_to_int_helper
std::string bool_to_int_transform(const clang::TranslationUnitDecl * tu_decl);

// helper function to initiate recursive call of bool_to_int_stmt
// on function body
std::pair<std::string, std::vector<std::string>> bool_to_int_helper(const clang::CompoundStmt * body,
                                                                    const std::string & pkt_name __attribute__((unused)));

// Recurse on stmt and continue simplifying using bool_to_int rules
std::string bool_to_int_stmt(const clang::Stmt * stmt);

#endif // BOOL_TO_INT_H_
