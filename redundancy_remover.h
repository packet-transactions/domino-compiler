#ifndef REDUNDANCY_REMOVER_H_
#define REDUNDANCY_REMOVER_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Entry point from FixedPointPass,
/// which immediately delegates to redundancy_remover_helper
std::string redundancy_remover_transform(const clang::TranslationUnitDecl * tu_decl);

// helper function to initiate recursive call of redundancy_remover_helper
// on function body
std::pair<std::string, std::vector<std::string>> redundancy_remover_helper(const clang::CompoundStmt * body,
                                                                           const std::string & pkt_name __attribute__((unused)));

// Recurse on stmt and continue simplifying by replacing all combinations
// of ParenExpr(ParenExpr) with a single ParenExpr
// and all combinations of CompoundStmt(CompoundStmt) with a single CompoundStmt
std::string redundancy_remover_stmt(const clang::Stmt * stmt);

#endif  // REDUNDANCY_REMOVER_H_
