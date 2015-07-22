#include "clang_utility_functions.h"
#include "state_var_decl_handler.h"

using namespace clang;
using namespace clang::ast_matchers;

void StateVarDeclHandler::run(const MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  // Handle only translation unit decls
  if (isa<TranslationUnitDecl>(decl)) {
    // Get all decls by dyn casting decl into a DeclContext
    for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
      assert(child_decl);
      if (isa<VarDecl>(child_decl)) {
        assert(child_decl->isDefinedOutsideFunctionOrMethod());

        // Prepend only global variables to output_,
        // remaining are prepended inside if_convert anyway
        // (the part of if_convert that handles DeclStmt)
        output_.insert(0, dyn_cast<VarDecl>(child_decl)->getType().getAsString() + " " + clang_value_decl_printer(dyn_cast<VarDecl>(child_decl)) + ";");
      }
    }
  }
}
