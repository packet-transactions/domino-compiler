#ifndef EXPR_FLATTENER_HANDLER_H_
#define EXPR_FLATTENER_HANDLER_H_

#include <ctime>
#include <cstdlib>

#include <utility>
#include <string>
#include <vector>
#include <set>

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

#include "unique_identifiers.h"

struct FlattenResult {
  std::string flat_expr;
  std::string new_defs;
  std::vector<std::string> new_decls;
};

/// Flatten expressions using temporaries so that every
/// statement is of the form x = y op z, where x, y, z are atomic
class ExprFlattenerHandler {
 public:
  /// Function supplied to SinglePass
  std::string transform(const clang::TranslationUnitDecl * tu_decl);

  /// Flatten function body
  std::pair<std::string, std::vector<std::string>> flatten_body(const clang::Stmt * function_body, const std::string & pkt_name) const;

 private:
  /// Flatten expression
  /// (http://www.cs.cornell.edu/projects/polyglot/api2/polyglot/visit/ExpressionFlattener.html)
  /// Returns a FlattenResult
  /// The first element is a flattened expression, one for which is_flat() returns true
  /// The second element is a vector of new definitions
  /// The third element is a vector of new packet variable declarations
  /// flatten maintains the invariant that the returned expression is flat,
  /// but it might create new definitions that aren't flat yet.
  /// We may need to run expr_flatten_prog multiple times until we reach a fixed point.
  FlattenResult flatten(const clang::Expr *expr, const std::string & pkt_name) const;

  /// Is expression flat?
  bool is_flat(const clang::Expr * expr) const;

  /// Is expression atomic?
  bool is_atomic_expr(const clang::Expr * expr) const;

  /// Flatten expr to atomic expression if it isn't already atomic
  /// , creating a temporary variable is required
  FlattenResult flatten_to_atomic_expr(const clang::Expr * expr, const std::string & pkt_name) const;

  /// Flatten conditional op by calling flatten_to_atomic_expr
  /// on its three constituents
  FlattenResult flatten_cond_op(const clang::ConditionalOperator * cond_op, const std::string & pkt_name) const;

  /// Flatten binary op by calling flatten_to_atomic_expr
  /// on the left and right halves
  FlattenResult flatten_bin_op(const clang::BinaryOperator * bin_op, const std::string & pkt_name) const;
  
  /// Flatten unary op by calling flatten_to_atomic_expr
  FlattenResult flatten_un_op(const clang::UnaryOperator * un_op, const std::string & pkt_name) const;

  /// Object that generates unique identifiers
  UniqueIdentifiers unique_identifiers_ = UniqueIdentifiers(std::set<std::string>());
};

#endif  // EXPR_FLATTENER_HANDLER_H_
