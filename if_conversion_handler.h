#ifndef IF_CONVERSION_HANDLER_H_
#define IF_CONVERSION_HANDLER_H_

#include <utility>
#include <string>
#include <vector>
#include <set>

#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

#include "unique_var_generator.h"

/// Rewrite if statements into ternary operators
/// and recursively get rid of all branches.
class IfConversionHandler {
 public:
  /// Constructor
  IfConversionHandler(const std::set<std::string> & init_set) : unique_var_gen_(init_set) {}

  /// Transform function
  std::pair<std::string, std::vector<std::string>> transform(const clang::Stmt * function_body, const std::string & pkt_name) const;

 private:
  /// if_convert current clang::Stmt
  /// Takes as input current if-converted program,
  /// current predicate, and the stmt itself (the AST)
  void if_convert(std::string & current_stream,
                  std::vector<std::string> & current_decls,
                  const std::string & predicate,
                  const clang::Stmt * stmt,
                  const std::string & pkt_name) const;

  /// If-convert an atomic (clang::BinaryOperator) statement
  /// with a conditional version of it
  std::string if_convert_atomic_stmt(const clang::BinaryOperator * stmt,
                                     const std::string & predicate) const;

  /// Unique variable generator
  UniqueVarGenerator unique_var_gen_;
};

#endif  // IF_CONVERSION_HANDLER_H_
