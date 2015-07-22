#ifndef IF_CONVERSION_HANDLER_H_
#define IF_CONVERSION_HANDLER_H_

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class IfConversionHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  /// Constructor
  IfConversionHandler() {}

  /// Callback whenever there's a match
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;

  /// Get output string
  std::string output() const { return output_; }

  /// Get vector of new variable declarations
  auto new_decls() const { return new_decls_; }
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

  /// String representing output
  std::string output_ = "";

  /// String representing new variable declarations
  std::vector<std::string> new_decls_ = {};
};

#endif  // IF_CONVERSION_HANDLER_H_
