#include "domino_to_group_domino_code_gen.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

#include <iostream>
#include <map>
#include <string>
#include <algorithm>

using namespace clang;

std::string DominoToGroupDominoCodeGenerator::ast_visit_transform(const clang::TranslationUnitDecl * tu_decl){
  std::string res;
  // TODO: Need to check if we have more than one packet func per tu_decl and report an error if so.
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) {
      //record body part first
      std::string body_part = ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      return res + "void " + dyn_cast<FunctionDecl>(decl)->getNameAsString() + "(" + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getType().getAsString() + " "
          + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getNameAsString() + "){\n" + body_part + "}";
    }else if (isa<VarDecl>(decl) || isa<RecordDecl>(decl)){
/*         if (isa<VarDecl>(decl)){
           std::cout << "Get Var Definition:" << dyn_cast<VarDecl>(decl)->getNameAsString() << std::endl;
           std::cout << "Get Var Type:" << dyn_cast<VarDecl>(decl)->getType().getAsString() << std::endl;
           std::cout << "Get Var Init:" << dyn_cast<VarDecl>(decl)->getEvaluatedValue() << std::endl;
         }
         else
           std::cout << "Get Record Definition:" << dyn_cast<RecordDecl>(decl)->getNameAsString() << std::endl;*/
         std::string str = clang_decl_printer(decl);
         for (std::map<std::string,std::string>::iterator it = c_to_sk.begin();it != c_to_sk.end();it++){
           size_t start_pos = str.find(it->first);
           if (start_pos == std::string::npos){
             continue;
           }else if (str[start_pos-1]==' '){
              str.replace(start_pos,it->first.length(),it->second);
           }
         }         
         res += str + ";\n";
    }else if ((isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl))))) {
             res += clang_decl_printer(decl) + "\n";
    }
  }
  assert_exception(false);
}

std::string DominoToGroupDominoCodeGenerator::ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr) {
  assert_exception(decl_ref_expr);
  std::string s = clang_stmt_printer(decl_ref_expr);
  std::map<std::string,std::string>::iterator it;
        it = c_to_sk.find(s);
        if (it != c_to_sk.end()){
           return c_to_sk[s];
        }else
           return s;
}

