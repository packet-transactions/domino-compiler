#ifndef CHIPMUNK_ANOTHERDOMINO_GENERATOR_H_
#define CHIPMUNK_ANOTHERDOMINO_GENERATOR_H_

#include "ast_visitor.h"
#include <string>
#include <map>
#include <algorithm>

class ChipmunkAnotherdominoGenerator : public AstVisitor {
 public:
  std::string ast_visit_transform(const clang::TranslationUnitDecl * tu_decl) override;
  // add another ast_visit_stmt_specially_for_if_condition
  std::string ast_visit_stmt_specially_for_if_condition(const clang::Stmt * stmt);
  std::string parameter_name;

  int round = 0;
  int rand = 0;
 protected:
  std::string ast_visit_member_expr(const clang::MemberExpr * member_expr) override;
  std::string ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr) override;
  std::string ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr) override;
  
  std::string ast_visit_if_stmt(const clang::IfStmt * if_stmt) override;
  std::string ast_visit_bin_op(const clang::BinaryOperator * bin_op) override;

  std::string ast_visit_record_decl(const clang::RecordDecl * record_decl, std::map<std::string, std::string> c_to_sk);

  int count_stateful = 0;
  int count_stateless = 0;  
  // TODO: This strange typedef is to address an issue with gcc:
  // https://stackoverflow.com/a/28803798/1152801
  typedef std::map<std::string, std::string> DominoToSketchRenamer;
  DominoToSketchRenamer c_to_sk = DominoToSketchRenamer();
};

#endif // CHIPMUNK_ANOTHERDOMINO_GENERATOR_H_
