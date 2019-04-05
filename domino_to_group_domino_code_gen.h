#ifndef DOMINO_TO_GROUP_DOMINO_CODE_GENERATOR_H_
#define DOMINO_TO_GROUP_DOMINO_CODE_GENERATOR_H_

#include "ast_visitor.h"
#include <map>
#include <string>

class DominoToGroupDominoCodeGenerator : public AstVisitor {
public:
  std::string
  ast_visit_transform(const clang::TranslationUnitDecl *tu_decl) override;
  // constructor
  DominoToGroupDominoCodeGenerator(
      std::map<std::string, std::string> origin_map) {
    for (std::map<std::string, std::string>::iterator it = origin_map.begin();
         it != origin_map.end(); it++)
      c_to_sk[it->first] = it->second;
  }

protected:
  std::string
  ast_visit_decl_ref_expr(const clang::DeclRefExpr *decl_ref_expr) override;

  // TODO: This strange typedef is to address an issue with gcc:
  // https://stackoverflow.com/a/28803798/1152801
  typedef std::map<std::string, std::string> DominoToGroupDomino;
  DominoToGroupDomino c_to_sk = DominoToGroupDomino();
};

#endif // DOMINO_TO_GROUP_DOMINO_CODE_GENERATOR_H_
