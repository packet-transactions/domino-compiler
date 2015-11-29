#ifndef AST_VISITOR_H_
#define AST_VISITOR_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"

class AstVisitor {
 public:
  /// Entry point from SinglePass,
  /// which immediately delegates to ast_visit_helper
  std::string ast_visit_transform(const clang::TranslationUnitDecl * tu_decl);

  /// Need destructor to be virtual and accessible, g++ whines otherwise
  virtual ~AstVisitor() {};

 /// Virtual methods for each node type
 protected:
  /// Visit a compound statement (braces in C)
  virtual std::string ast_visit_comp_stmt(const clang::CompoundStmt * comp_stmt);

  /// Visit an if statement (if, else-if, in C)
  virtual std::string ast_visit_if_stmt(const clang::IfStmt * if_stmt);

  /// Visit a binary operator (a = b, a + b, a * b)
  virtual std::string ast_visit_bin_op(const clang::BinaryOperator * bin_op);

  /// Visit a conditional operator (a ? b : c, not including GNU's extension)
  virtual std::string ast_visit_cond_op(const clang::ConditionalOperator * cond_op);

  /// Visit a function call expression (f(1, 2, 3))
  virtual std::string ast_visit_func_call(const clang::CallExpr * call_expr);

  /// Visit a member expression (packet variables of the form pkt.x)
  virtual std::string ast_visit_member_expr(const clang::MemberExpr * member_expr);

  /// Visit a state variable expression (any access to global state)
  virtual std::string ast_visit_decl_ref_expr(const clang::DeclRefExpr * decl_ref_expr);

  /// Visit integer constants (-1L, 2U, 123, etc.)
  virtual std::string ast_visit_integer_literal(const clang::IntegerLiteral * integer_literal);

  /// Visit an array subscript node (a[3])
  virtual std::string ast_visit_array_subscript_expr(const clang::ArraySubscriptExpr * array_subscript_expr);

  /// Visit a unary operator (-2 or ! false)
  virtual std::string ast_visit_un_op(const clang::UnaryOperator * un_op);

  /// Visit a node (clang::Stmt) recursively
  /// This is protected so that derived classes can call it recursively,
  /// but not virtual because the grammar is fixed and ast_visit simply
  /// captures a walk down the AST
  std::string ast_visit(const clang::Stmt * stmt);

 private:
  /// Wrapper around recursive function ast_visit_helper
  std::pair<std::string, std::vector<std::string>> ast_visit_helper(const clang::CompoundStmt * body,
                                                                    const std::string & pkt_name __attribute__((unused)));
};

#endif // AST_VISITOR_H_
