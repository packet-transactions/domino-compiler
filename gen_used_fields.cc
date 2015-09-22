#include <utility>

#include "gen_used_fields.h"

#include "third_party/assert_exception.h"

#include "clang_utility_functions.h"
#include "pkt_func_transform.h"
#include "util.h"

using namespace clang;

std::string gen_used_field_transform(const clang::TranslationUnitDecl * tu_decl) {
  for (const auto * child_decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    assert_exception(child_decl);
    if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      const auto * function_decl = dyn_cast<FunctionDecl>(child_decl);

      // Extract function signature
      assert_exception(function_decl->getNumParams() >= 1);
      const auto * pkt_param = function_decl->getParamDecl(0);
      const auto pkt_type  = function_decl->getParamDecl(0)->getType().getAsString();
      const auto pkt_name = clang_value_decl_printer(pkt_param);

      // Transform function body
      return gen_used_field_body(dyn_cast<clang::CompoundStmt>(function_decl->getBody()), pkt_name);
    }
  }
  assert_exception(false);
  return "";
}

std::string gen_used_field_body(const clang::CompoundStmt * function_body,
                                const std::string & pkt_name __attribute__((unused))) {
  const auto var_list = gen_var_list(function_body,
                                     {{VariableType::PACKET, true},
                                      {VariableType::STATE_SCALAR, false},
                                      {VariableType::STATE_ARRAY, false}});
  std::string ret = "";
  for (const auto & var : var_list) {
    std::vector<std::string> splits = split(var, "\\.");
    assert_exception(splits.size() == 2);
    ret += splits.at(1) + "\n";
  }
  return ret;
}
