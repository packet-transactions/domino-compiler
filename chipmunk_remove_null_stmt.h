#ifndef CHIPMUNK_NULLSTMT_REMOVER_H_
#define CHIPMUNK_NULLSTMT_REMOVER_H_

#include <algorithm>
#include <map>
#include <string>

#include "ast_visitor.h"

class ChipmunkNullstmtRemover : public AstVisitor {
public:
  std::string
  ast_visit_remove_nullstmt(const clang::TranslationUnitDecl *tu_decl);

protected:
  // TODO: This strange typedef is to address an issue with gcc:
  // https://stackoverflow.com/a/28803798/1152801
  typedef std::map<std::string, std::string> DominoToSketchRenamer;
  DominoToSketchRenamer c_to_sk = DominoToSketchRenamer();
};

#endif // CHIPMUNK_NULLSTMT_REMOVER_H_
