#include "domino_to_group_domino_code_gen.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>

#include "clang_utility_functions.h"
#include "third_party/assert_exception.h"

using namespace clang;

std::string DominoToGroupDominoCodeGenerator::ast_visit_transform(
    const clang::TranslationUnitDecl *tu_decl) {
  std::string res;
  // TODO: Need to check if we have more than one packet func per tu_decl and
  // report an error if so.
  for (const auto *decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and
        (is_packet_func(dyn_cast<FunctionDecl>(decl)))) {
      // record body part first
      std::string body_part =
          ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      return res + "void " + dyn_cast<FunctionDecl>(decl)->getNameAsString() +
             "(" +
             dyn_cast<FunctionDecl>(decl)
                 ->getParamDecl(0)
                 ->getType()
                 .getAsString() +
             " " +
             dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getNameAsString() +
             "){\n" + body_part + "}";
    } else if (isa<VarDecl>(decl) || isa<RecordDecl>(decl)) {
      if (isa<VarDecl>(decl)) {
        // Pay special attention to the definition without initialization
        if (clang_decl_printer(decl).find('=') == std::string::npos) {
          std::string
              stateful_var_name; // stateful_var_name store the name which
                                 // should appear in the definition part
          std::map<std::string, std::string>::iterator it;
          it = c_to_sk.find(dyn_cast<VarDecl>(decl)->getNameAsString());
          if (it != c_to_sk.end()) {
            stateful_var_name =
                c_to_sk[dyn_cast<VarDecl>(decl)->getNameAsString()];
          } else {
            stateful_var_name = dyn_cast<VarDecl>(decl)->getNameAsString();
          }
          res += "int " + stateful_var_name + ";\n";
          continue;
        }
        // dyn_cast<VarDecl>(decl)->getDefinition() gets the var_name i.e int
        // count = 0; --> count
        std::string var_name =
            clang_value_decl_printer(dyn_cast<VarDecl>(decl)->getDefinition());
        // dyn_cast<VarDecl>(decl)->getInit() get initial value i.e int count =
        // 0; --> the initializer is 0
        std::string init_val =
            clang_stmt_printer(dyn_cast<VarDecl>(decl)->getInit());
        // To see whether the stateful_var is an array or not
        std::size_t found = init_val.find('{');
        if (found != std::string::npos) {
          var_name += '[';
        }
        // To see whether this stateful_vars has appeared in the function
        // body
        std::string stateful_var_name; // stateful_var_name store the name which
                                       // should appear in the definition part
        std::map<std::string, std::string>::iterator it;
        it = c_to_sk.find(var_name);
        if (it != c_to_sk.end()) {
          stateful_var_name = c_to_sk[var_name];
        } else {
          stateful_var_name = var_name;
        }
        // Return the result ## need some special help for the array_vars
        res += "int " + stateful_var_name + " = " + init_val + ";\n";
      } else if (isa<RecordDecl>(decl)) {
        // dyn_cast<RecordDecl>(decl)->getNameAsString() get the name of the
        // struct
        res +=
            "struct " + dyn_cast<RecordDecl>(decl)->getNameAsString() + "{\n";
        for (const auto *field_decl : dyn_cast<DeclContext>(decl)->decls()) {
          std::string pkt_vars =
              dyn_cast<FieldDecl>(field_decl)->getNameAsString();
          // To see whether this stateless_vars has appeared in the
          // function body
          std::string
              stateless_var_name; // stateful_var_name store the name which
                                  // should appear in the definition part
          std::map<std::string, std::string>::iterator it;
          it = c_to_sk.find(pkt_vars);
          if (it != c_to_sk.end()) {
            stateless_var_name = c_to_sk[pkt_vars];
          } else {
            stateless_var_name = pkt_vars;
          }
          // dyn_cast<FieldDecl>(field_decl)->getNameAsString() get the name of
          // pkt_vars
          res += "    int " + stateless_var_name + ";\n";
        }
        res += "};\n";
      }

      /*
               std::string str = clang_decl_printer(decl);
               for (std::map<std::string,std::string>::iterator it =
         c_to_sk.begin();it != c_to_sk.end();it++){ size_t start_pos =
         str.find(it->first); if (start_pos == std::string::npos){ continue;
                 }else if (str[start_pos-1]==' '){
                    str.replace(start_pos,it->first.length(),it->second);
                 }
               }
               res += str + ";\n";*/
    } else if ((isa<FunctionDecl>(decl) and
                (not is_packet_func(dyn_cast<FunctionDecl>(decl))))) {
      res += clang_decl_printer(decl) + "\n";
    }
  }
  assert_exception(false);
}

std::string DominoToGroupDominoCodeGenerator::ast_visit_decl_ref_expr(
    const clang::DeclRefExpr *decl_ref_expr) {
  assert_exception(decl_ref_expr);
  std::string s = clang_stmt_printer(decl_ref_expr);
  std::map<std::string, std::string>::iterator it;
  it = c_to_sk.find(s);
  if (it != c_to_sk.end()) {
    return c_to_sk[s];
  } else
    return s;
}
