#include "banzai_code_generator.h"

#include <string>
#include <tuple>

#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"

#include "third_party/temp_file.hh"
#include "third_party/system_runner.hh"

#include "util.h"
#include "config.h"
#include "set_idioms.h"
#include "clang_utility_functions.h"

using namespace clang;

const std::string BanzaiCodeGenerator::PACKET_IDENTIFIER = "packet";
const std::string BanzaiCodeGenerator::STATE_IDENTIFIER  = "state";

int BanzaiCodeGenerator::get_order(const Decl * decl) const {
  if (isa<VarDecl>(decl)) return 1;
  else if (isa<FunctionDecl>(decl) and (not is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 2;
  else if (isa<FunctionDecl>(decl) and (is_packet_func(dyn_cast<FunctionDecl>(decl)))) return 3;
  else if (isa<RecordDecl>(decl)) return 4;
  else if (isa<TypedefDecl>(decl)) return 5;
  else {assert(false); return -1; }
}

std::string BanzaiCodeGenerator::rewrite_into_banzai_ops(const clang::Stmt * stmt) const {
  assert(stmt);

  if(isa<CompoundStmt>(stmt)) {
    std::string ret;
    for (const auto & child : stmt->children())
      ret += rewrite_into_banzai_ops(child) + ";";
    return ret;
  } else if (isa<IfStmt>(stmt)) {
    const auto * if_stmt = dyn_cast<IfStmt>(stmt);
    std::string ret;
    ret += "if (" + rewrite_into_banzai_ops(if_stmt->getCond()) + ") {" + rewrite_into_banzai_ops(if_stmt->getThen()) + "; }";
    if (if_stmt->getElse() != nullptr) {
      ret += "else {" + rewrite_into_banzai_ops(if_stmt->getElse()) + "; }";
    }
    return ret;
  } else if (isa<BinaryOperator>(stmt)) {
    const auto * bin_op = dyn_cast<BinaryOperator>(stmt);
    return rewrite_into_banzai_ops(bin_op->getLHS()) + std::string(bin_op->getOpcodeStr()) + rewrite_into_banzai_ops(bin_op->getRHS());
  } else if (isa<ConditionalOperator>(stmt)) {
    const auto * cond_op = dyn_cast<ConditionalOperator>(stmt);
    return   "(" + rewrite_into_banzai_ops(cond_op->getCond()) + ") ? ("
             + rewrite_into_banzai_ops(cond_op->getTrueExpr()) + ") : ("
             + rewrite_into_banzai_ops(cond_op->getFalseExpr()) + ")";
  } else if (isa<MemberExpr>(stmt)) {
    const auto * member_expr = dyn_cast<MemberExpr>(stmt);
    // All packet fields are of the type p(...) in banzai
    // N.B. the Banzai code overloads the () operator.
    return   PACKET_IDENTIFIER + "(\"" + clang_value_decl_printer(member_expr->getMemberDecl()) + "\")";
  } else if (isa<DeclRefExpr>(stmt)) {
    // All state variables are of the type s(...) in banzai
    // N.B. Again by overloading the () operator
    const auto * decl_expr = dyn_cast<DeclRefExpr>(stmt);
    return   STATE_IDENTIFIER + "(\"" + clang_value_decl_printer(decl_expr->getDecl()) + "\")";
  } else if (isa<IntegerLiteral>(stmt)) {
    return clang_stmt_printer(stmt);
  } else if (isa<ParenExpr>(stmt)) {
    return "(" + rewrite_into_banzai_ops(dyn_cast<ParenExpr>(stmt)->getSubExpr()) + ")";
  } else if (isa<ImplicitCastExpr>(stmt)) {
    return rewrite_into_banzai_ops(dyn_cast<ImplicitCastExpr>(stmt)->getSubExpr());
  } else {
    throw std::logic_error("rewrite_into_banzai_ops cannot handle stmt of type " + std::string(stmt->getStmtClassName()));
  }
}

std::tuple<BanzaiCodeGenerator::BanzaiAtomDefinition,
           BanzaiCodeGenerator::BanzaiPacketFieldSet,
           BanzaiCodeGenerator::BanzaiAtomName>
BanzaiCodeGenerator::rewrite_into_banzai_atom(const clang::Stmt * stmt)  const {
  const auto atom_name = unique_identifiers_.get_unique_identifier("atom");
  return std::make_tuple(
         "void " +
         atom_name +
         "(Packet & " + PACKET_IDENTIFIER + " __attribute__((unused)), State & " + STATE_IDENTIFIER + " __attribute__((unused))) {\n" +
         rewrite_into_banzai_ops(stmt) + "\n }",

         gen_var_list(stmt, VariableType::PACKET),
         atom_name);
}

BanzaiCodeGenerator::BanzaiProgram BanzaiCodeGenerator::transform_translation_unit(const clang::TranslationUnitDecl * tu_decl) const {
  // Accumulate all declarations
  std::vector<const Decl*> all_decls;
  for (const auto * decl : dyn_cast<DeclContext>(tu_decl)->decls())
    all_decls.emplace_back(decl);

  // Sort all_decls
  std::sort(all_decls.begin(),
            all_decls.end(),
            [this] (const auto * decl1, const auto * decl2)
            { return this->get_order(decl1) < this->get_order(decl2); });

  // Storage for returned string
  std::string ret;
  // Storage for initial values of all state variables
  std::map<std::string, uint32_t> init_values;

  for (const auto * child_decl : all_decls) {
    assert(child_decl);
    if (isa<VarDecl>(child_decl)) {
      const auto * var_decl = dyn_cast<VarDecl>(child_decl);
      // Forbid certain constructs
      if (not var_decl->hasInit()) throw std::logic_error("All state variables must have an initializer in domino: " + clang_value_decl_printer(var_decl)+ " doesn't");
      if (init_values.find(clang_value_decl_printer(var_decl)) != init_values.end()) throw std::logic_error("Reinitializing " + clang_value_decl_printer(var_decl) + " to " + clang_stmt_printer(var_decl->getInit()) + " not permitted");
      if (not isa<IntegerLiteral>(var_decl->getInit())) throw std::logic_error("Only integers can be used to initialize state variables: " + clang_value_decl_printer(var_decl) + " uses " + clang_stmt_printer(var_decl->getInit()));
      init_values[clang_value_decl_printer(var_decl)] = static_cast<uint32_t>(std::stoul(clang_stmt_printer(var_decl->getInit())));
    } else if ((isa<FunctionDecl>(child_decl) and (not is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) or
        isa<RecordDecl>(child_decl)) {
      // Just quench these, don't emit them
    } else if (isa<FunctionDecl>(child_decl) and (is_packet_func(dyn_cast<FunctionDecl>(child_decl)))) {
      const auto return_tuple = rewrite_into_banzai_atom(dyn_cast<FunctionDecl>(child_decl)->getBody());

      // Add include files for banzai (the equivalent of a target ABI)
      ret += "#include \"packet.h\"\n";
      ret += "#include \"atom.h\"\n";
      ret += "#include \"pipeline.h\"\n";

      // Add an extern C flank to get around name mangling
      ret += "extern \"C\"{\n";

      // Generate atom definition
      ret += std::get<0>(return_tuple);

      // Generate test_fields for banzai
      ret += "PacketFieldSet test_fields(";
      if (not std::get<1>(return_tuple).empty()) {
        ret += "{";
        for (const auto & field : std::get<1>(return_tuple)) {
          ret += "\"" + field + "\",";
        }
        ret.back() = '}';
      } else {
        ret += "{}";
      }
      ret += ");";

      // Generate initial values for all state variables
      std::string init_state_str = "FieldContainer(std::map<FieldContainer::FieldName, uint32_t>";
      if (not init_values.empty()) {
        init_state_str += "{";
        for (const auto & state_var_pair : init_values) {
          init_state_str += "{\"" + state_var_pair.first + "\", " + std::to_string(state_var_pair.second) + "},";
        }
        init_state_str.back() = '}'; // to close std::map constructor's initializer list
      } else {
        init_state_str += "{}";
      }
      init_state_str += ")"; // to close FieldContainer constructor's left parenthesis

      // Generate test_pipeline for banzai
      ret += "Pipeline test_pipeline{{Atom(" + std::get<2>(return_tuple) + ", " + init_state_str + ")}};";

      // Close extern C declaration
      ret += "}";

    } else {
      assert(isa<TypedefDecl>(child_decl));
    }
  }

  // Turn banzai C++ code into a library
  return gen_lib_as_string(ret);
}

BanzaiCodeGenerator::BanzaiLibString BanzaiCodeGenerator::gen_lib_as_string(const BanzaiCodeGenerator::BanzaiProgram & banzai_program) const {
  // TempFile to hold banzai_program
  TempFile banzai_prog_file("/tmp/banzai_prog", ".cc");
  banzai_prog_file.write(banzai_program);

  // Compile banzai_file into a .o file
  TempFile object_file("/tmp/banzai_obj", ".o");
  run({GPLUSPLUS, "-std=c++14", "-pedantic", "-Wconversion", "-Wsign-conversion", "-Wall", "-Wextra", "-Weffc++", "-Werror", "-fno-default-inline", "-g", "-c", banzai_prog_file.name(), "-fPIC", "-DPIC", "-o", object_file.name()});

  // Turn that into a shared library
  TempFile library_file("/tmp/libbanzai", ".so");
  run({GPLUSPLUS, "-shared", "-o", library_file.name(), object_file.name()});

  // Return library file binary as a string
  // (hopefully doesn't bork the terminal)
  return file_to_str(library_file.name());
}

std::set<std::string> BanzaiCodeGenerator::gen_var_list(const clang::Stmt * stmt, const BanzaiCodeGenerator::VariableType & var_type) const {
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
