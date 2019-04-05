#ifndef RENAME_DOMINO_CODE_GENERATOR_H_
#define RENAME_DOMINO_CODE_GENERATOR_H_

#include <map>
#include <string>

#include "ast_visitor.h"

class RenameDominoCodeGenerator : public AstVisitor {
public:
  std::string
  ast_visit_transform(const clang::TranslationUnitDecl *tu_decl) override;

protected:
  std::string
  ast_visit_member_expr(const clang::MemberExpr *member_expr) override;
  std::string ast_visit_array_subscript_expr(
      const clang::ArraySubscriptExpr *array_subscript_expr) override;
  std::string
  ast_visit_decl_ref_expr(const clang::DeclRefExpr *decl_ref_expr) override;
  //Print the map content
  void print_map();

  int count_stateful = 0;
  int count_stateless = 0;

  // TODO: This strange typedef is to address an issue with gcc:
  // https://stackoverflow.com/a/28803798/1152801
  typedef std::map<std::string, std::string> DominoToRenameDomino;
  DominoToRenameDomino c_to_sk = DominoToRenameDomino();
};

#endif // RENAME_DOMINO__CODE_GENERATOR_H_
