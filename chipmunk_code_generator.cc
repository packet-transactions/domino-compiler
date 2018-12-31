#include "chipmunk_code_generator.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

using namespace clang;

std::string ChipmunkCodeGenerator::ast_visit_member_expr(const clang::MemberExpr * member_expr) {
  assert_exception(member_expr);
  return clang_stmt_printer(member_expr->getBase()) + "." + clang_value_decl_printer(member_expr->getMemberDecl()) + "_00";
}
