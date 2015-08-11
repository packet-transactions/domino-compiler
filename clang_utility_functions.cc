#include "clang_utility_functions.h"

#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/LangOptions.h"
#include "clang/AST/PrettyPrinter.h"

using namespace clang;

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

std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl) {
  std::set<std::string> identifiers = {};
  assert(decl != nullptr);

  // Get all decls by dyn casting decl into a DeclContext
  for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
    assert(child_decl);
    assert(child_decl->isDefinedOutsideFunctionOrMethod());
    if (isa<RecordDecl>(child_decl)) {
      // add current fields in struct to identifiers
      for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
        identifiers.emplace(dyn_cast<FieldDecl>(field_decl)->getName());
    } else if (isa<FunctionDecl>(child_decl)) {
      // add function name
      identifiers.emplace(dyn_cast<FunctionDecl>(child_decl)->getName());
      // add all function parameters
      for (const auto * parm_decl : dyn_cast<FunctionDecl>(child_decl)->parameters()) {
        identifiers.emplace(dyn_cast<ParmVarDecl>(parm_decl)->getName());
      }
    } else if (isa<ValueDecl>(child_decl)) {
      // add state variable name
      identifiers.emplace(dyn_cast<ValueDecl>(child_decl)->getName());
    } else {
      // We can't remove this for some reason.
      assert(isa<TypedefDecl>(child_decl));
    }
  }
  return identifiers;
}
