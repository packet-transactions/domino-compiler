#ifndef PACKET_VARIABLE_CREATOR_H_
#define PACKET_VARIABLE_CREATOR_H_

#include <set>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class PacketVariableCreator : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  /// Called at the beginning to populate var_names_
  /// with the current set of packet variables so that we
  /// can generate unique names afterwards
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;

  /// Return var_names_
  auto output() const { return var_names_; }

 private:
  /// Set of variable names
  std::set<std::string> var_names_ = {};
};

#endif  // PACKET_VARIABLE_CREATOR_H_
