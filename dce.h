#ifndef DCE_H_
#define DCE_H_

#include <set>
#include <string>
#include <utility>
#include <map>
#include "clang/AST/Expr.h"


/// Dead Code Elimination: Eliminates defines (LHS assignment ops) 
/// without any uses after SSA. This also needs to be scheduled as a fixedpoint pass
/// as eliminating one assignment statement may lead to the elimination of another.
std::string dce_transform(const clang::TranslationUnitDecl * tu_decl);

/// Main function for DCE.
std::pair<std::string, std::vector<std::string>> dce_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));


#endif // DCE_H_
