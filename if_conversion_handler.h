#ifndef IF_CONVERSION_HANDLER_H_
#define IF_CONVERSION_HANDLER_H_

#include <utility>
#include <string>
#include <vector>
#include <set>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

#include "unique_identifiers.h"

/// Rewrite if statements into ternary operators
/// and recursively get rid of all branches.
class IfConversionHandler {
 public:
  /// Transform function itself, entry point to SinglePass
  std::string transform(const clang::TranslationUnitDecl * tu_decl);

  /// If convert function body
  std::pair<std::string, std::vector<std::string>> if_convert_body(const clang::Stmt * function_body, const std::string & pkt_name) const;

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

  /// Unique identifier generator
  UniqueIdentifiers unique_identifiers_ = UniqueIdentifiers(std::set<std::string>());
};

#endif  // IF_CONVERSION_HANDLER_H_
