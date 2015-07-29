#ifndef EXPR_FLATTENER_HANDLER_H_
#define EXPR_FLATTENER_HANDLER_H_

#include <utility>
#include <string>
#include <vector>
#include <set>
#include "clang/AST/AST.h"
#include "unique_var_generator.h"
#include <ctime>
#include <cstdlib>

struct FlattenResult {
  std::string flat_expr;
  std::string new_defs;
  std::vector<std::string> new_decls;
};

class ExprFlattenerHandler {
 public:
  /// Constructor
  ExprFlattenerHandler(const std::set<std::string> & t_var_set) : unique_var_gen_(t_var_set) {}

  /// Transform function
  std::pair<std::string, std::vector<std::string>> transform(const clang::Stmt * function_body, const std::string & pkt_name) const;

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

  /// Is expression an atom?
  bool is_atom(const clang::Expr * expr) const;

  /// Flatten expr to atom if it isn't already an atom
  /// , creating a temporary variable is required
  FlattenResult flatten_to_atom(const clang::Expr * expr, const std::string & pkt_name) const;

  /// Flatten conditional op by calling flatten_to_atom
  /// on its three constituents
  FlattenResult flatten_cond_op(const clang::ConditionalOperator * cond_op, const std::string & pkt_name) const;

  /// Flatten binary op by calling flatten_to_atom
  /// on the left and right halves
  FlattenResult flatten_bin_op(const clang::BinaryOperator * bin_op, const std::string & pkt_name) const;

  /// Unique variable generator
  UniqueVarGenerator unique_var_gen_;
};

#endif  // EXPR_FLATTENER_HANDLER_H_
