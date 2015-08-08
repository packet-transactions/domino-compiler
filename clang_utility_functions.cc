#include "clang_utility_functions.h"

#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/LangOptions.h"
#include "clang/AST/PrettyPrinter.h"

std::string clang_stmt_printer(const clang::Stmt * stmt) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  stmt->printPretty(rso, nullptr, Policy);
  return str;
}

std::string clang_value_decl_printer(const clang::ValueDecl * value_decl) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  value_decl->printName(rso);
  return str;
}

std::string clang_decl_printer(const clang::Decl * decl) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  decl->print(rso);
  return str;
}

bool is_packet_func(const clang::FunctionDecl * func_decl) {
  // Not sure what we would get out of functions with zero args
  assert(func_decl->getNumParams() >= 1);
  return func_decl->getNumParams() == 1
         and func_decl->getParamDecl(0)->getType().getAsString() == "struct Packet";
}
