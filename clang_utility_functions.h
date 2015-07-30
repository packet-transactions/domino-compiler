#ifndef CLANG_UTILITY_FUNCTIONS_H_
#define CLANG_UTILITY_FUNCTIONS_H_

#include <string>
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/AST.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

std::string clang_stmt_printer(const clang::Stmt * stmt);

std::string clang_value_decl_printer(const clang::ValueDecl * value_decl);

std::string clang_decl_printer(const clang::Decl * decl);

bool is_packet_func(const clang::FunctionDecl * func_decl);


#endif  // CLANG_UTILITY_FUNCTIONS_H_
