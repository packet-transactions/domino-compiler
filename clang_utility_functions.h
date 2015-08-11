#ifndef CLANG_UTILITY_FUNCTIONS_H_
#define CLANG_UTILITY_FUNCTIONS_H_

#include <set>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

/// General puprose printer for clang stmts
std::string clang_stmt_printer(const clang::Stmt * stmt);

/// Print clang value decl
std::string clang_value_decl_printer(const clang::ValueDecl * value_decl);

/// Print all kinds of clang declarations
/// This is best used when we want to pass through certain statements unchanged.
std::string clang_decl_printer(const clang::Decl * decl);

/// Is this a packet function: does it have struct Packet as an argument
bool is_packet_func(const clang::FunctionDecl * func_decl);

/// Return the current set of identifiers
/// so that we can generate unique names afterwards
std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl);

#endif  // CLANG_UTILITY_FUNCTIONS_H_
