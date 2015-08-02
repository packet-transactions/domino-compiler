#ifndef PKT_FUNC_TRANSFORM_H_
#define PKT_FUNC_TRANSFORM_H_

#include <utility>
#include <string>
#include <vector>
#include <functional>
#include "clang/AST/AST.h"

/// Tranform a translation unit by modifying packet functions
/// alone. Pass through the rest as such without modifications.
std::string pkt_func_transform(const clang::TranslationUnitDecl * tu_decl,
                               const std::function<std::pair<std::string, std::vector<std::string>>(const clang::CompoundStmt *)> & func_body_transform);

#endif  // PKT_FUNC_TRANSFORM_H_
