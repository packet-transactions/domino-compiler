#include "clang_utility_functions.h"

#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "clang/Basic/LangOptions.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"

#include "third_party/assert_exception.h"
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
  assert_exception(func_decl->getNumParams() >= 1);
  return func_decl->getNumParams() == 1
         and func_decl->getParamDecl(0)->getType().getAsString() == "struct Packet";
}

std::string replace_var_helper(const Expr * expr, const std::map<std::string, std::string> & repl_map) {
  const std::string var_name = clang_stmt_printer(expr);
  if (repl_map.find(var_name) != repl_map.end()) {
    return repl_map.at(var_name);
  } else {
    return var_name;
  }
}

std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl, const VariableTypeSelector & var_selector) {
  std::set<std::string> identifiers = {};
  assert_exception(decl != nullptr);

  // Get all decls by dyn casting decl into a DeclContext
  for (const auto * child_decl : dyn_cast<DeclContext>(decl)->decls()) {
    assert_exception(child_decl);
    assert_exception(child_decl->isDefinedOutsideFunctionOrMethod());
    if (isa<RecordDecl>(child_decl)) {
      // add current fields in struct to identifiers
      if (var_selector.at(VariableType::PACKET)) {
        for (const auto * field_decl : dyn_cast<DeclContext>(child_decl)->decls())
          identifiers.emplace(dyn_cast<FieldDecl>(field_decl)->getName());
      }
    } else if (isa<FunctionDecl>(child_decl)) {
      if (var_selector.at(VariableType::FUNCTION_PARAMETER)) {
        // add function name
        identifiers.emplace(dyn_cast<FunctionDecl>(child_decl)->getName());
        // add all function parameters
        for (const auto * parm_decl : dyn_cast<FunctionDecl>(child_decl)->parameters()) {
          identifiers.emplace(dyn_cast<ParmVarDecl>(parm_decl)->getName());
        }
      }
    } else if (isa<VarDecl>(child_decl)) {
      const auto * underlying_type = dyn_cast<VarDecl>(child_decl)->getType().getTypePtrOrNull();
      if (isa<ConstantArrayType>(underlying_type)) {
        if (var_selector.at(VariableType::STATE_ARRAY)) {
          identifiers.emplace(dyn_cast<VarDecl>(child_decl)->getName());
        }
      } else if (isa<BuiltinType>(underlying_type) and dyn_cast<BuiltinType>(underlying_type)->isInteger()) {
        if (var_selector.at(VariableType::STATE_SCALAR)) {
          identifiers.emplace(dyn_cast<VarDecl>(child_decl)->getName());
        }
      } else {
        throw std::logic_error("We don't support this: state variable "
                               + clang_value_decl_printer(dyn_cast<VarDecl>(child_decl))
                               + " has type " + std::string(dyn_cast<VarDecl>(child_decl)->getName()));
      }
    } else {
      // We can't remove TypedefDecl from the AST for some reason.
      assert_exception(isa<TypedefDecl>(child_decl));
    }
  }
  return identifiers;
}

std::set<std::string> gen_var_list(const Stmt * stmt, const VariableTypeSelector & var_selector) {
  // Recursively scan stmt to generate a set of strings representing
  // either packet fields or state variables used within stmt
  assert_exception(stmt);
  std::set<std::string> ret;
  if (isa<CompoundStmt>(stmt)) {
    for (const auto & child : stmt->children()) {
      ret = ret + gen_var_list(child, var_selector);
    }
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    if (if_stmt->getElse() != nullptr) {
      return gen_var_list(if_stmt->getCond(), var_selector) + gen_var_list(if_stmt->getThen(), var_selector) + gen_var_list(if_stmt->getElse(), var_selector);
    } else {
      return gen_var_list(if_stmt->getCond(), var_selector) + gen_var_list(if_stmt->getThen(), var_selector);
    }
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return gen_var_list(bin_op->getLHS(), var_selector) + gen_var_list(bin_op->getRHS(), var_selector);
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return gen_var_list(cond_op->getCond(), var_selector) + gen_var_list(cond_op->getTrueExpr(), var_selector) + gen_var_list(cond_op->getFalseExpr(), var_selector);
  } else if (isa<MemberExpr>(stmt)) {
    return (var_selector.at(VariableType::PACKET))
           ? std::set<std::string>{clang_stmt_printer(stmt)}
           : std::set<std::string>();
  } else if (isa<DeclRefExpr>(stmt)) {
    return (var_selector.at(VariableType::STATE_SCALAR))
           ? std::set<std::string>{clang_stmt_printer(stmt)}
           : std::set<std::string>();
  } else if (isa<ArraySubscriptExpr>(stmt)) {
    // We return array name here, not array name subscript index.
    // We assume that the array name is a unique identifier because
    // per-packet we only ever access one address in the array.
    if (var_selector.at(VariableType::STATE_ARRAY)) {
      return std::set<std::string>{clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(stmt)->getBase())};
    } else if (var_selector.at(VariableType::PACKET)) {
      return std::set<std::string>{clang_stmt_printer(dyn_cast<ArraySubscriptExpr>(stmt)->getIdx())};
    } else {
      return std::set<std::string>();
    }
  } else if (isa<IntegerLiteral>(stmt) or isa<NullStmt>(stmt)) {
    return std::set<std::string>();
  } else if (isa<ParenExpr>(stmt)) {
    return gen_var_list(dyn_cast<ParenExpr>(stmt)->getSubExpr(), var_selector);
  } else if (isa<UnaryOperator>(stmt)) {
    const auto * un_op = dyn_cast<UnaryOperator>(stmt);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return gen_var_list(un_op->getSubExpr(), var_selector);
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return gen_var_list(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr(), var_selector);
  } else if (isa<CallExpr>(stmt)) {
    const auto * call_expr = dyn_cast<CallExpr>(stmt);
    std::set<std::string> ret;
    for (const auto * child : call_expr->arguments()) {
      const auto child_uses = gen_var_list(child, var_selector);
      ret = ret + child_uses;
    }
    return ret;
  } else {
    throw std::logic_error("gen_var_list cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}

std::string generate_scalar_func_def(const FunctionDecl * func_decl) {
  // Yet another C quirk. Adding a semicolon after a function definition
  // is caught by -pedantic, not adding a semicolon after a function declaration
  // without a definition is not permitted in C :)
  assert_exception(func_decl);
  assert_exception(not is_packet_func(func_decl));
  const bool has_body = func_decl->hasBody();
  return clang_decl_printer(func_decl) + (has_body ? "" : ";");
}

std::string gen_pkt_fields(const TranslationUnitDecl * tu_decl) {
  std::string ret = "";
  for (const auto & field : identifier_census(tu_decl,
                                              {{VariableType::PACKET, true},
                                               {VariableType::STATE_SCALAR,  false},
                                               {VariableType::FUNCTION_PARAMETER, false},
                                               {VariableType::STATE_ARRAY, false}})) {
    ret += field + "\n";
  }
  return ret;
}

std::string replace_vars(const clang::Expr * expr,
                         const std::map<std::string, std::string> & repl_map,
                         const VariableTypeSelector & var_selector) {
  assert_exception(expr);
  if (isa<ParenExpr>(expr)) {
    return "(" + replace_vars(dyn_cast<ParenExpr>(expr)->getSubExpr(), repl_map, var_selector) + ")";
  } else if (isa<CastExpr>(expr)) {
    return replace_vars(dyn_cast<CastExpr>(expr)->getSubExpr(), repl_map, var_selector);
  } else if (isa<UnaryOperator>(expr)) {
    const auto * un_op = dyn_cast<UnaryOperator>(expr);
    assert_exception(un_op->isArithmeticOp());
    const auto opcode_str = std::string(UnaryOperator::getOpcodeStr(un_op->getOpcode()));
    assert_exception(opcode_str == "!");
    return opcode_str + replace_vars(un_op->getSubExpr(), repl_map, var_selector);
  } else if (isa<ConditionalOperator>(expr)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(expr);

    const auto cond_str = replace_vars(cond_op->getCond(), repl_map, var_selector);
    const auto true_str = replace_vars(cond_op->getTrueExpr(), repl_map, var_selector);
    const auto false_str = replace_vars(cond_op->getFalseExpr(), repl_map, var_selector);

    return cond_str + " ? " + true_str + " : " + false_str;
  } else if (isa<BinaryOperator>(expr)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(expr);
    const auto lhs_str = replace_vars(bin_op->getLHS(), repl_map, var_selector);
    const auto rhs_str = replace_vars(bin_op->getRHS(), repl_map, var_selector);
    return lhs_str + std::string(BinaryOperator::getOpcodeStr(bin_op->getOpcode())) + rhs_str;
  } else if (isa<DeclRefExpr>(expr)) {
    if (var_selector.at(VariableType::STATE_SCALAR)) return replace_var_helper(expr, repl_map);
    else return clang_stmt_printer(expr);
  } else if (isa<MemberExpr>(expr)) {
    if (var_selector.at(VariableType::PACKET)) return replace_var_helper(expr, repl_map);
    else return clang_stmt_printer(expr);
  } else if (isa<ArraySubscriptExpr>(expr)) {
    const auto * array_op = dyn_cast<ArraySubscriptExpr>(expr);
    if (var_selector.at(VariableType::STATE_ARRAY)) return replace_var_helper(expr, repl_map);
    else if (var_selector.at(VariableType::PACKET)) return clang_stmt_printer(array_op->getBase()) + "[" + replace_vars(array_op->getIdx(), repl_map, var_selector) + "]";
    else return clang_stmt_printer(expr);
  } else if (isa<CallExpr>(expr)) {
    const auto * call_expr = dyn_cast<CallExpr>(expr);
    std::string ret = clang_stmt_printer(call_expr->getCallee()) + "(";
    for (const auto * child : call_expr->arguments()) {
      const auto child_str = replace_vars(child, repl_map, var_selector);
      ret += child_str + ",";
    }
    ret.back() = ')';
    return ret;
  } else if (isa<IntegerLiteral>(expr)){
    return clang_stmt_printer(expr);
  } else {
    throw std::logic_error("replace_vars cannot handle expr " + std::string(clang_stmt_printer(expr)) + " of type " + std::string(expr->getStmtClassName()));
  }
}

bool is_in_ssa(const CompoundStmt * compound_stmt) {
  std::set<std::string> assigned_vars;
  for (const auto * child : compound_stmt->children()) {
    assert_exception(isa<BinaryOperator>(child));
    const auto * bin_op = dyn_cast<BinaryOperator>(child);
    assert_exception(bin_op->isAssignmentOp());
    const auto * lhs = bin_op->getLHS()->IgnoreParenImpCasts();
    const auto pair = assigned_vars.emplace(clang_stmt_printer(lhs));
    if (pair.second == false) {
      return false;;
    }
  }
  return true;
}
