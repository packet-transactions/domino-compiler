#include <algorithm>
#include "pkt_func_transform.h"
#include "clang_utility_functions.h"

using namespace clang;

int get_order(const Decl * decl) {
  if (isa<VarDecl>(decl)) return 1;
  else if (isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 2;
  else if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 3;
  else if (isa<RecordDecl>(decl)) return 4;
  else if (isa<TypedefDecl>(decl)) return 5;
  else {assert(false); return -1; }
}

std::string pkt_func_transform(const TranslationUnitDecl * tu_decl,
                               const std::function<std::pair<std::string, std::vector<std::string>>(const CompoundStmt *)> & func_body_transform) {
  // Accumulate all declarations
  std::vector<const Decl*> all_decls;
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls())
    all_decls.emplace_back(decl);

  // Sort all_decls
  std::sort(all_decls.begin(),
            all_decls.end(),
            [] (const auto * decl1, const auto * decl2)
            { return get_order(decl1) < get_order(decl2); });

  // Loop through sorted vector of declarations
  std::string state_var_str = "";
  std::string scalar_func_str = "";
  std::string pkt_func_str = "";
  std::string record_decl_str = "";
  for (const auto * child_decl : all_decls) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl)) {
      state_var_str += clang_decl_printer(child_decl) + ";";
    } else if ((isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl))))) {
      scalar_func_str += clang_decl_printer(child_decl) + ";";
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      const auto * function_decl = dyn_cast<FunctionDecl>(child_decl);
      const auto transformed_body = func_body_transform(dyn_cast<CompoundStmt>(function_decl->getBody())).first;

      // Append function body to signature
      assert(function_decl->getNumParams() >= 1);
      const auto * pkt_param = function_decl->getParamDecl(0);
      const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
      const auto pkt_name = clang_value_decl_printer(pkt_param);

      // Get transformed_body string
      pkt_func_str += function_decl->getReturnType().getAsString() + " " +
                      function_decl->getNameInfo().getName().getAsString() +
                      "( " + pkt_type + " " +  pkt_name + ") { " +
                      transformed_body + "}\n";
    } else if (isa<RecordDecl>(child_decl)) {
      record_decl_str += clang_decl_printer(child_decl) + ";";
    }
  }
  return state_var_str + scalar_func_str + record_decl_str + pkt_func_str;
}
