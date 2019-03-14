#ifndef RENAME_DOMINO_TEST_CODE_GENERATOR_H_
#define RENAME_DOMINO_TEST_CODE_GENERATOR_H_

#include "ast_visitor.h"
#include <string>
#include <map>

class RenameDominoCodeGenerator : public AstVisitor {
 public:
  std::string ast_visit_transform_mutator(const clang::TranslationUnitDecl * tu_decl);//,std::map<std::string,std::string> state_to_group);
  //constructor
  RenameDominoCodeGenerator(std::map<std::string,std::string> origin_map){
    for (std::map<std::string,std::string>::iterator it = origin_map.begin();it!=origin_map.end();it++)
      c_to_sk[it->first] = it -> second;    
  }

 protected:
  std::string ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr) override;
  
  int count_stateful = 0;
  int count_stateless = 0;

  // TODO: This strange typedef is to address an issue with gcc:
  // https://stackoverflow.com/a/28803798/1152801
  typedef std::map<std::string, std::string> DominoToRenameDomino;
  DominoToRenameDomino c_to_sk = DominoToRenameDomino();
};

#endif // RENAME_DOMINO_TEST_CODE_GENERATOR_H_
