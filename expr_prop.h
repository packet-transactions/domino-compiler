#ifndef EXPR_PROP_H_
#define EXPR_PROP_H_

#include <string>
#include <utility>

#include "clang/AST/AST.h"

/// Entry point from SinglePass to expression
/// propagater, calls expr_prop_fn_body immediately
std::string expr_prop_transform(const clang::TranslationUnitDecl * tu_decl);

/// Expr propagation: replace y = b + c; a = y;
/// with y=b+c; a=b+c; In some sense we are inverting
/// common subexpression elimination
std::pair<std::string, std::vector<std::string>> expr_prop_fn_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

#endif  // EXPR_PROP_H_
