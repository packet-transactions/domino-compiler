#ifndef CONST_PROP_H_
#define CONST_PROP_H_

#include <set>
#include <string>
#include <utility>
#include <map>
#include "clang/AST/Expr.h"


/// Constant propagation
std::string const_prop_transform(const clang::TranslationUnitDecl * tu_decl);

/// Main function for constant propagation that rewrites function body by
/// identifying and eliminating variables whos RHSes are constants.
/// This pass must be scheduled as a FixedPointPass, in order to iteratively
/// eliminate any remaining constexprs.
std::pair<std::string, std::vector<std::string>> const_prop_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));


#endif // CONST_PROP_H_
