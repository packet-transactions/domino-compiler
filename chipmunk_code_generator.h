#ifndef CHIPMUNK_CODE_GENERATOR_H_
#define CHIPMUNK_CODE_GENERATOR_H_

#include "ast_visitor.h"

class ChipmunkCodeGenerator : public AstVisitor {
 protected:
  std::string ast_visit_member_expr(const clang::MemberExpr * member_expr) override;

};

#endif // CHIPMUNK_CODE_GENERATOR_H_
