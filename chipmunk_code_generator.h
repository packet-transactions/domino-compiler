#ifndef CHIPMUNK_CODE_GENERATOR_H_
#define CHIPMUNK_CODE_GENERATOR_H_

#include "ast_visitor.h"
#include <map>
#include <string>

class ChipmunkCodeGenerator : public AstVisitor {
 protected:
  std::string ast_visit_member_expr(const clang::MemberExpr * member_expr) override;
  std::string ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr) override;
  void print_map(); //cout the content of map 
  
  int count_stateful = 0;
  int count_stateless = 0;
  std::map<std::string, std::string> c_to_sk; 
};

#endif // CHIPMUNK_CODE_GENERATOR_H_
