#ifndef SSA_H_
#define SSA_H_

#include <string>
#include <utility>
#include <set>

#include "clang/AST/Decl.h"

// Static Single-Assignment form for function body, excluding the final write
// in the write epilogue to state variables. This guarantees that each packet
// variable is assigned exactly once. If it is assigned more than once, perform
// simple renaming. SSA is very simple in domino because we have no branches and no phi nodes.
std::string ssa_transform(const clang::TranslationUnitDecl * tu_decl);

/// Helper function that does most of the heavy lifting in SSA, by rewriting the function body into SSA form.
std::pair<std::string, std::vector<std::string>> ssa_rewrite_fn_body(const clang::CompoundStmt * function_body, const std::string & pkt_name, const std::set<std::string> & id_set);

#endif  // SSA_H_
