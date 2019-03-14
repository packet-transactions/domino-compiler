#include "rename_domino_code_generator.h"

#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"

#include <iostream>
#include <map>
#include <string>
#include <algorithm>

using namespace clang;

std::string RenameDominoCodeGenerator::ast_visit_transform(const clang::TranslationUnitDecl * tu_decl) {
  // TODO: Need to check if we have more than one packet func per tu_decl and report an error if so.
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) {
      //record body part first
      std::string body_part = ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      return "void func(struct Packet p) {\n" + body_part + "\n}";
    }
  }
  assert_exception(false);
}

std::string RenameDominoCodeGenerator::ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr) {
  assert_exception(decl_ref_expr);
  std::string s = clang_stmt_printer(decl_ref_expr);
  std::map<std::string,std::string>::iterator it;
        it = c_to_sk.find(s);
        if (it == c_to_sk.end()){
            std::string name;
            //stateless
	    if (s.find('.')!=std::string::npos){
		// Should never get here.
		assert_exception(false);
	    }
	    else{
                name = "state_" + std::to_string(count_stateful);
                count_stateful++;
		c_to_sk[s] = name;
	    }
        }
  return c_to_sk[s];
}

std::string RenameDominoCodeGenerator::ast_visit_member_expr(const clang::MemberExpr * member_expr) {
  assert_exception(member_expr);
  std::string s = clang_stmt_printer(member_expr);
  std::map<std::string,std::string>::iterator it;
        it = c_to_sk.find(s);
        if (it == c_to_sk.end()){
            std::string name;
            //stateless
            if (s.find('.')!=std::string::npos && s.find('[')==std::string::npos){
                name = "p.pkt_" + std::to_string(count_stateless);
                count_stateless++;
            }
            else{
                // Should never get here.
                assert_exception(false);
            }
            c_to_sk[s] = name;
        }
  return c_to_sk[s];
}

std::string RenameDominoCodeGenerator::ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr){
  assert_exception(array_subscript_expr);
  std::string s = clang_stmt_printer(array_subscript_expr);

  std::map<std::string,std::string>::iterator it;
        it = c_to_sk.find(s);
        if (it == c_to_sk.end()){
            std::string name;
  	    if (s.find('[')==std::string::npos){
		// Should never get here.
		assert_exception(false);
	    }else{
            //stateless
            name = "state_" + std::to_string(count_stateful);
            count_stateful++;
            c_to_sk[s] = name;
	    }
        }
  return c_to_sk[s];
}
