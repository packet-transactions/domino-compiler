#include <iostream>
#include "clang_utility_functions.h"
#include "func_decl_pass_through.h"

using namespace clang;
using namespace clang::ast_matchers;

void FuncDeclPassThrough::run(const MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  // Handle only translation unit decls
  if (isa<TranslationUnitDecl>(decl)) {
    // Get all decls by dyn casting decl into a DeclContext
    for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
      assert(child_decl);
      if (isa<FunctionDecl>(child_decl)) {
        assert(child_decl->isDefinedOutsideFunctionOrMethod());

        // Check if this is a function declaration that has
        // exactly one Packet argument, someone else will take care of it
        const auto * func_decl = dyn_cast<FunctionDecl>(child_decl);
        if (is_packet_func(func_decl)) {
          ;
        } else {
          output_ += clang_decl_printer(child_decl) + ";";
        }
      }
    }
  }
}
