#ifndef PACKET_DECL_HANDLER_H_
#define PACKET_DECL_HANDLER_H_

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

class PacketDeclHandler : public clang::ast_matchers::MatchFinder::MatchCallback {
 public:
  PacketDeclHandler(const std::vector<std::string> & t_new_decls) : new_decls_(t_new_decls) {}
  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult & t_result) override;
  auto output() const { return output_; }
 private:
  std::string output_ = "";
  std::vector<std::string> new_decls_ = {};
};

#endif  // PACKET_DECL_HANDLER_H_
