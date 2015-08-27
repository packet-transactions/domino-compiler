#ifndef DESUGAR_COMPOUND_ASSIGNMENT_H_ 
#define DESUGAR_COMPOUND_ASSIGNMENT_H_ 

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"

/// Entry point from SinglePass,
/// which immediately delegates to desugar_compound_assignment
std::string desugar_compound_assignment_transform(const clang::TranslationUnitDecl * tu_decl);

/// Wrapper around recursive function desugar_compound_assignment
std::pair<std::string, std::vector<std::string>> desugar_compound_assignment_helper(const clang::CompoundStmt * body,
                                                                                    const std::string & pkt_name __attribute__((unused)));

/// get_underlying_op from a compound assignment operator
clang::BinaryOperator::Opcode get_underlying_op(const clang::BinaryOperator::Opcode & comp_asgn_op);

/// Desugar compound assignment, by replacing:
/// E1 op= E2 with E1 = E1 op E2
/// This is almost correct (quoting from cppreference.com)
/// "except that the expression E1 is evaluated only once
///and that it behaves as a single operation with respect
///to indeterminately-sequenced function calls (e.g. in f(a+= b, g()),
/// the += is either not started at all or is
///completed as seen from inside g())."
/// I don't think that's reasonable code anyway, so for
/// now I am ignoring it (TODO).
std::string desugar_compound_assignments(const clang::Stmt * stmt);

#endif // DESUGAR_COMPOUND_ASSIGNMENT_H_
