#include "pkt_func_transform.h"
#include "clang_utility_functions.h"

using namespace clang;

std::string pkt_func_transform(const TranslationUnitDecl * tu_decl, const std::function<std::string(const CompoundStmt *)> & func_body_transform) {
  std::string ret;
  // Loop through all declarations within the translation unit decl
  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl) or
        isa<RecordDecl>(child_decl) or
        (isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl))))) {
      ret += clang_decl_printer(child_decl) + ";";
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      const auto * function_decl = dyn_cast<FunctionDecl>(child_decl);

      const auto transformed_body = func_body_transform(dyn_cast<CompoundStmt>(function_decl->getBody()));

      // Append function body to signature
      assert(function_decl->getNumParams() >= 1);
      const auto * pkt_param = function_decl->getParamDecl(0);
      const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
      const auto pkt_name = clang_value_decl_printer(pkt_param);

      // Get transformed_body string
      ret += function_decl->getReturnType().getAsString() + " " +
             function_decl->getNameInfo().getName().getAsString() +
             "( " + pkt_type + " " +  pkt_name + ") { " +
             transformed_body + "}\n";
    }
  }
  return ret;
}
