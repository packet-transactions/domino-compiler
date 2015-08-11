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
/// statement is of the form x = y op z, where x, y, z are atoms
class ExprFlattenerHandler {
 public:
  /// Function supplied to SinglePass
  static std::string transform(const clang::TranslationUnitDecl * tu_decl);

  /// Flatten function body
  static std::pair<std::string, std::vector<std::string>> flatten_body(const clang::Stmt * function_body, const std::string & pkt_name, const std::set<std::string> & id_set);

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
  static FlattenResult flatten(const clang::Expr *expr, const std::string & pkt_name, UniqueIdentifiers & unique_identifiers);

  /// Is expression flat?
  static bool is_flat(const clang::Expr * expr);

  /// Is expression an atom?
  static bool is_atom(const clang::Expr * expr);

  /// Flatten expr to atom if it isn't already an atom
  /// , creating a temporary variable is required
  static FlattenResult flatten_to_atom(const clang::Expr * expr, const std::string & pkt_name, UniqueIdentifiers & unique_identifiers);

  /// Flatten conditional op by calling flatten_to_atom
  /// on its three constituents
  static FlattenResult flatten_cond_op(const clang::ConditionalOperator * cond_op, const std::string & pkt_name, UniqueIdentifiers & unique_identifiers);

  /// Flatten binary op by calling flatten_to_atom
  /// on the left and right halves
  static FlattenResult flatten_bin_op(const clang::BinaryOperator * bin_op, const std::string & pkt_name, UniqueIdentifiers & unique_identifiers);
};

#endif  // EXPR_FLATTENER_HANDLER_H_
