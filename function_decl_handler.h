#ifndef FUNCTION_DECL_HANDLER_H_
#define FUNCTION_DECL_HANDLER_H_

#include <string>
#include <set>
#include "clang/Lex/Lexer.h"
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include "clang_utility_functions.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

class FunctionDeclHandler : public MatchFinder::MatchCallback {
 public:
  /// Constructor: Pass Refactoring tool as argument
  FunctionDeclHandler(Replacements & t_replace, const std::set<std::string> & t_decl_strings) : Replace(t_replace), decl_strings_(t_decl_strings) {}

  /// Callback whenever there's a match
  virtual void run(const MatchFinder::MatchResult &Result) override {
    const FunctionDecl *function_decl_expr = Result.Nodes.getNodeAs<clang::FunctionDecl>("functionDecl");
    assert(function_decl_expr != nullptr);

    // Concatenate all declarations
    std::string all_decls = "";
    for (const auto & decl : decl_strings_)
      all_decls += "  " + decl;

    // Now, create replacement text
    auto start_loc = Lexer::getLocForEndOfToken(function_decl_expr->getBody()->getLocStart(), 0, *Result.SourceManager, Result.Context->getLangOpts());
    Replacement Rep(*(Result.SourceManager), start_loc, 0,
                    all_decls);

    // Insert into this Replace
    Replace.insert(Rep);
  }

 private:
  Replacements & Replace;
  const std::set<std::string> decl_strings_;
};

#endif  // FUNCTION_DECL_HANDLER_H_
