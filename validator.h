#ifndef VALIDATOR_H_
#define VALIDATOR_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"

/// Entry point from SinglePass,
/// which immediately delegates to validator_helper
std::string validator_transform(const clang::TranslationUnitDecl * tu_decl);

/// Wrapper around recursive function validator_helper
std::pair<std::string, std::vector<std::string>> validator_helper(const clang::CompoundStmt * body,
                                                                  const std::string & pkt_name __attribute__((unused)));

/// Validate function body
std::string validator(const clang::Stmt * stmt);

#endif // VALIDATOR_H_
