#ifndef PKT_FUNC_TRANSFORM_H_
#define PKT_FUNC_TRANSFORM_H_

#include <utility>
#include <string>
#include <vector>
#include <functional>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

/// Order of parsing declarations (state, packets, state functions, packet functions)
int get_order(const clang::Decl * decl);

/// Convenience typedef for a function that transforms a function body
typedef std::function<std::pair<std::string, std::vector<std::string>>(const clang::CompoundStmt *, const std::string & pkt_name)> FuncBodyTransform;

/// Tranform a translation unit by modifying packet functions
/// alone. Pass through the rest as such without modifications.
std::string pkt_func_transform(const clang::TranslationUnitDecl * tu_decl,
                               const FuncBodyTransform & func_body_transform);

#endif  // PKT_FUNC_TRANSFORM_H_
