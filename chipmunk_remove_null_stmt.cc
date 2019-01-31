#include "chipmunk_remove_null_stmt.h"
#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

#include <iostream>
#include <map>
#include <string>
#include <algorithm>

using namespace std::placeholders;
using namespace clang;

std::string ChipmunkNullstmtRemover::ast_visit_remove_nullstmt(const clang::TranslationUnitDecl * tu_decl){

  std::string res;
  // TODO: Need to check if we have more than one packet func per tu_decl and report an error if so.
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) {
      //record body part first
      std::string body_part = ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      return res + "void " + dyn_cast<FunctionDecl>(decl)->getNameAsString() + "(" + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getType().getAsString() + " "
          + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getNameAsString() + "){\n" + body_part + "}";
    }else if (isa<VarDecl>(decl) || isa<RecordDecl>(decl)){
      res += clang_decl_printer(decl) + ";\n";
    }else if ((isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl))))) { 
       res += clang_decl_printer(decl) + "\n";
    }
  }
  assert_exception(false);
}
