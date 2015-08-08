#include "packet_variable_census.h"

#include "clang_utility_functions.h"

using namespace clang;

std::set<std::string> packet_variable_census(const clang::TranslationUnitDecl * decl) {
  std::set<std::string> var_names = {};
  assert(decl != nullptr);

  // Get all decls by dyn casting decl into a DeclContext
  for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
    assert(child_decl);
    assert(child_decl->isDefinedOutsideFunctionOrMethod());
    if (isa<RecordDecl>(child_decl)) {
      // add current fields in struct to var_names_
      for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
        var_names.emplace(clang_value_decl_printer(dyn_cast<ValueDecl>(field_decl)));
    }
  }
  return var_names;
}
