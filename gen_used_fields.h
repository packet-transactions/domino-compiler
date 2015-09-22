#ifndef GEN_USED_FIELDS_H_
#define GEN_USED_FIELDS_H_

#include <set>
#include <vector>
#include <string>
#include "clang/AST/Stmt.h"
#include "clang/AST/Decl.h"

/// Entry point from SinglePass,
/// immediately delegating to gen_used_field_body
std::string gen_used_field_transform(const clang::TranslationUnitDecl * tu_decl);

/// Scan function body and generate list of used fields as a string
/// This is used by jayhawk for test packet generation
std::string gen_used_field_body(const clang::CompoundStmt * function_body, const std::string & pkt_name __attribute__((unused)));

#endif  // GEN_USED_FIELDS_H_
