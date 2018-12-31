#include "chipmunk_code_generator.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

using namespace clang;

std::string ChipmunkCodeGenerator::ast_visit_member_expr(const clang::MemberExpr * member_expr) {
  assert_exception(member_expr);
  return AstVisitor::ast_visit_member_expr(member_expr);
}
