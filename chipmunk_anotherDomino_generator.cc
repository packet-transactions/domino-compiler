#include "chipmunk_anotherDomino_generator.h"
#include "third_party/assert_exception.h"
#include "clang_utility_functions.h"
#include "pkt_func_transform.h"

#include <iostream>
#include <map>
#include <string>
#include <algorithm>

using namespace std::placeholders;
using namespace clang;

std::string ChipmunkAnotherdominoGenerator::ast_visit_record_decl(const clang::RecordDecl * record_decl, std::map<std::string, std::string> c_to_sk) {
  assert_exception(record_decl);
  std::string ret = "struct " + record_decl->getNameAsString() + "{\n";
  for (const auto * decl : record_decl->getDefinition()->decls()){
      ret += clang_decl_printer(decl)+";\n";
  }
  if (rand == 1){
    for(std::map<std::string,std::string>::const_iterator it = c_to_sk.begin();it != c_to_sk.end(); ++it){
        ret += "int " + it->second + ";\n";
    } 
  }
  ret += "}; \n\n";
  return ret;
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_stmt_specially_for_if_condition(const Stmt * stmt) {
  assert_exception(stmt);
  std::string ret;
  if(isa<CompoundStmt>(stmt)) {
    return ast_visit_comp_stmt(dyn_cast<CompoundStmt>(stmt));
  } else if (isa<IfStmt>(stmt)) {
    return ast_visit_if_stmt(dyn_cast<IfStmt>(stmt));
  } else if (isa<BinaryOperator>(stmt)) {
    const BinaryOperator * bin_op = dyn_cast<BinaryOperator>(stmt);
    const auto opcode = bin_op->getOpcode();
    if ((opcode == clang::BinaryOperatorKind::BO_LAnd or opcode == clang::BinaryOperatorKind::BO_Or) and rand == 3)
      return ast_visit_stmt_specially_for_if_condition(bin_op->getRHS()) + std::string(bin_op->getOpcodeStr()) + ast_visit_stmt_specially_for_if_condition(bin_op->getLHS());
    else
      return ast_visit_stmt_specially_for_if_condition(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + ast_visit_stmt_specially_for_if_condition(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    return ast_visit_cond_op(dyn_cast<ConditionalOperator>(stmt));
  } else if (isa<MemberExpr>(stmt)) {
    return ast_visit_member_expr(dyn_cast<MemberExpr>(stmt));
  } else if (isa<DeclRefExpr>(stmt)) {
    return ast_visit_decl_ref_expr(dyn_cast<DeclRefExpr>(stmt));
  } else if (isa<IntegerLiteral>(stmt)) {
    return ast_visit_integer_literal(dyn_cast<IntegerLiteral>(stmt));
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    return ast_visit_array_subscript_expr(dyn_cast<ArraySubscriptExpr>(stmt));
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + ast_visit_stmt(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<UnaryOperator>(stmt)) {
    return ast_visit_un_op(dyn_cast<UnaryOperator>(stmt));
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return ast_visit_implicit_cast(dyn_cast<ImplicitCastExpr>(stmt));
  } else if (isa<CallExpr>(stmt)) {
    return ast_visit_func_call(dyn_cast<CallExpr>(stmt));
  } else if (isa<NullStmt>(stmt)) {
    return ";";
  } else {
    throw std::logic_error("ast_visit error: the statement\n"
                           + clang_stmt_printer(stmt)
                           + "\nis of type "
                           + std::string(stmt->getStmtClassName())
                           + ", which isn't allowed in domino");
  }
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_transform(const clang::TranslationUnitDecl * tu_decl) {

  // First pass to establish the c_to_sk map
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl))))
      //record body part first
      ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      parameter_name = dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getNameAsString();
  }

  std::string res;
  // TODO: Need to check if we have more than one packet func per tu_decl and report an error if so.
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls()) {
    if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) {
      //record body part first
      std::string body_part = ast_visit_stmt(dyn_cast<FunctionDecl>(decl)->getBody());
      //use stateless variable to replace stateful variable
      std::string definition;
      std::string assignment_back;
      if (rand == 1){
        for(std::map<std::string,std::string>::const_iterator it = c_to_sk.begin();it != c_to_sk.end(); ++it){
          definition += "p." + it->second + " = " + it->first + ";\n";
          assignment_back += it->first + " = p." + it->second + ";\n";
        }  
      }
      res += "void " + dyn_cast<FunctionDecl>(decl)->getNameAsString() + "(" + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getType().getAsString() + " "
          + dyn_cast<FunctionDecl>(decl)->getParamDecl(0)->getNameAsString() + "){\n" + definition + body_part + assignment_back + "}";
      return res;
  //now is func p 
      
    }else if (isa<VarDecl>(decl) || ((isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl))))) ){ 
	   res += clang_decl_printer(decl) + ";\n";
    }else if ( isa<RecordDecl>(decl) ){
     res += ast_visit_record_decl(dyn_cast<RecordDecl>(decl), c_to_sk) + "\n";
  }
}
  assert_exception(false);
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_if_stmt(const IfStmt * if_stmt) {
  assert_exception(if_stmt);
  std::string ret;
  if (if_stmt->getElse() != nullptr && rand == 2) {
    ret += "if (!(" + ast_visit_stmt_specially_for_if_condition(if_stmt->getCond()) + ")) {" + ast_visit_stmt(if_stmt->getElse()) + "; }";
    ret += "else if (" + ast_visit_stmt_specially_for_if_condition(if_stmt->getCond()) + ") {" + ast_visit_stmt(if_stmt->getThen()) + "; }";
    return ret;
  }else{
    ret += "if (" + ast_visit_stmt_specially_for_if_condition(if_stmt->getCond()) + ") {" + ast_visit_stmt(if_stmt->getThen()) + "; }";
    return ret;
  }
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_bin_op(const BinaryOperator * bin_op) {
  assert_exception(bin_op);
  return ast_visit_stmt(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + ast_visit_stmt(bin_op->getRHS());
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr) {
  assert_exception(decl_ref_expr);
  std::string s = clang_stmt_printer(decl_ref_expr);
  if (rand == 1){
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
                name = "tmp_" + std::to_string(round) + "_" + std::to_string(count_stateful);
                count_stateful++;
                c_to_sk[s] = name;
            }
        }
  
    return parameter_name + c_to_sk[s];  
  }else{
    return s;
  }
  
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_member_expr(const clang::MemberExpr * member_expr) {
  assert_exception(member_expr);
  std::string s = clang_stmt_printer(member_expr);
  return s;
}

std::string ChipmunkAnotherdominoGenerator::ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr){
  assert_exception(array_subscript_expr);
  std::string s = clang_stmt_printer(array_subscript_expr);
  if (rand == 1){
    std::map<std::string,std::string>::iterator it;
          it = c_to_sk.find(s);
          if (it == c_to_sk.end()){
              std::string name;
              if (s.find('[')==std::string::npos){
                  // Should never get here.
                  assert_exception(false);
              }else{
              //stateless
              name = "tmp_" + std::to_string(round) + "_" + std::to_string(count_stateful);
              count_stateful++;
              c_to_sk[s] = name;
              }
          }
    return parameter_name + c_to_sk[s];
  }else{
    return s;
  }
}

