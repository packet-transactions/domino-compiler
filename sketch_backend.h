#ifndef SKETCH_BACKEND_H_
#define SKETCH_BACKEND_H_

#include <vector>
#include <string>
#include <set>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

/// String representing a packet field like "p.x"
typedef std::string PktField;

/// String represent a scalar variable name like "x"
typedef std::string ScalarVarName;

/// Transform code blocks (or) equivalently, strongly connected
/// components into SKETCH specifications.
/// SKETCH specifications can then be fed to the SKETCH compiler to
/// synthesize configurations for atoms
/// (represented as sketches with holes).
std::string sketch_backend_transform(const clang::TranslationUnitDecl * tu_decl);

/// Transfrom a Stmt representing a function body
/// into a SKETCH specification.
std::string create_sketch_spec(const clang::Stmt * function_body);

/// Collect all state variable expressions recursively from a clang::Stmt
/// TODO: This is very similar to gen_var_list with a few minor differences.
/// All of these are symptoms that we need a recursive traversal mechansim
/// that is generic and spans all these AST traversal functions.
std::set<std::string> collect_state_vars(const clang::Stmt * function_body);

#endif  // SKETCH_BACKEND_H_
