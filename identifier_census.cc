#include "identifier_census.h"

#include "clang_utility_functions.h"

using namespace clang;

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
