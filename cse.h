#ifndef CSE_H_
#define CSE_H_

#include <string>
#include <utility>
#include "clang/AST/Expr.h"

// Entry point to Common Subexpression Elimination (CSE)
std::string cse_transform(const clang::TranslationUnitDecl * tu_decl);

// Main function for CSE from a function body
std::pair<std::string, std::vector<std::string>> cse_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__ ((unused)));

#endif  // CSE_H_
