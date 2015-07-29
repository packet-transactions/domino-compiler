#include "clang_utility_functions.h"
#include "packet_variable_creator.h"

using namespace clang;
using namespace clang::ast_matchers;

void PacketVariableCreator::run(const MatchFinder::MatchResult & t_result) {
  const auto * decl = t_result.Nodes.getNodeAs<Decl>("decl");
  assert(decl != nullptr);

  // Handle only translation unit decls
  if (isa<TranslationUnitDecl>(decl)) {
    // Get all decls by dyn casting decl into a DeclContext
    for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
      assert(child_decl);
      assert(child_decl->isDefinedOutsideFunctionOrMethod());
      if (isa<RecordDecl>(child_decl)) {
        // add current fields in struct to var_names_
        for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
          var_names_.emplace(clang_value_decl_printer(dyn_cast<ValueDecl>(field_decl)));
      }
    }
  }
}

