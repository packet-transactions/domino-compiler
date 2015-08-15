#include "clang_utility_functions.h"

#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/LangOptions.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"

#include "set_idioms.h"

using namespace clang;

std::string clang_stmt_printer(const clang::Stmt * stmt) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  stmt->printPretty(rso, nullptr, Policy);
  return str;
}

std::string clang_value_decl_printer(const clang::ValueDecl * value_decl) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  value_decl->printName(rso);
  return str;
}

std::string clang_decl_printer(const clang::Decl * decl) {
  // Required for pretty printing
  clang::LangOptions LangOpts;
  LangOpts.CPlusPlus = true;
  clang::PrintingPolicy Policy(LangOpts);

  std::string str;
  llvm::raw_string_ostream rso(str);
  decl->print(rso);
  return str;
}

bool is_packet_func(const clang::FunctionDecl * func_decl) {
  // Not sure what we would get out of functions with zero args
  assert(func_decl->getNumParams() >= 1);
  return func_decl->getNumParams() == 1
         and func_decl->getParamDecl(0)->getType().getAsString() == "struct Packet";
}

std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl) {
  std::set<std::string> identifiers = {};
  assert(decl != nullptr);

  // Get all decls by dyn casting decl into a DeclContext
  for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
    assert(child_decl);
    assert(child_decl->isDefinedOutsideFunctionOrMethod());
    if (isa<RecordDecl>(child_decl)) {
      // add current fields in struct to identifiers
      for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
        identifiers.emplace(dyn_cast<FieldDecl>(field_decl)->getName());
    } else if (isa<FunctionDecl>(child_decl)) {
      // add function name
      identifiers.emplace(dyn_cast<FunctionDecl>(child_decl)->getName());
      // add all function parameters
      for (const auto * parm_decl : dyn_cast<FunctionDecl>(child_decl)->parameters()) {
        identifiers.emplace(dyn_cast<ParmVarDecl>(parm_decl)->getName());
      }
    } else if (isa<ValueDecl>(child_decl)) {
      // add state variable name
      identifiers.emplace(dyn_cast<ValueDecl>(child_decl)->getName());
    } else {
      // We can't remove this for some reason.
      assert(isa<TypedefDecl>(child_decl));
    }
  }
  return identifiers;
}

std::set<std::string> gen_var_list(const Stmt * stmt, const VariableType & var_type) {
  // Recursively scan stmt to generate a set of strings representing
  // either packet fields or state variables used within stmt
  assert(stmt);
  std::set<std::string> ret;
  if (isa<CompoundStmt>(stmt)) {
    for (const auto & child : stmt->children()) {
      ret = ret + gen_var_list(child, var_type);
    }
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    if (if_stmt->getElse() != nullptr) {
      return gen_var_list(if_stmt->getCond(), var_type) + gen_var_list(if_stmt->getThen(), var_type) + gen_var_list(if_stmt->getElse(), var_type);
    } else {
      return gen_var_list(if_stmt->getCond(), var_type) + gen_var_list(if_stmt->getThen(), var_type);
    }
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return gen_var_list(bin_op->getLHS(), var_type) + gen_var_list(bin_op->getRHS(), var_type);
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return gen_var_list(cond_op->getCond(), var_type) + gen_var_list(cond_op->getTrueExpr(), var_type) + gen_var_list(cond_op->getFalseExpr(), var_type);
  } else if (isa<MemberExpr>(stmt)) {
    const auto * packet_var_expr = dyn_cast<MemberExpr>(stmt);
    return var_type == VariableType::PACKET ? std::set<std::string>{clang_value_decl_printer(packet_var_expr->getMemberDecl())} : std::set<std::string>();
  } else if (isa<DeclRefExpr>(stmt)) {
    const auto * state_var_expr = dyn_cast<DeclRefExpr>(stmt);
    return var_type == VariableType::STATE ? std::set<std::string>{clang_stmt_printer(state_var_expr)} : std::set<std::string>();
  } else if (isa<IntegerLiteral>(stmt)) {
    return std::set<std::string>();
  } else if (isa<ParenExpr>(stmt)) {
    return gen_var_list(dyn_cast<ParenExpr>(stmt)->getSubExpr(), var_type);
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return gen_var_list(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr(), var_type);
  } else {
    throw std::logic_error("gen_var_list cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}
