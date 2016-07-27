#ifndef SKETCH_BACKEND_H_
#define SKETCH_BACKEND_H_

#include <vector>
#include <string>
#include <set>
#include <map>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"

/// String representing a packet field like "p.x"
typedef std::string PktField;

/// String represent a scalar variable name like "x"
typedef std::string ScalarVarName;

struct PacketFieldPair {
  std::set<PktField> incoming_fields = {};
  std::set<PktField> defined_fields  = {};
};

/// Extract packet fields from function body
/// i.e. non-array subscript packet fields that are
/// either "external" arguments to the codelet (incoming_fields) or
/// are defined by the codelet body (defined_fields)
PacketFieldPair extract_packet_fields(const clang::Stmt * function_body);

/// Core logic that coalesces arguments in a codelet if they are correlated
std::string coalesce_args(const clang::Stmt * function_body,
                          const std::map<PktField, std::set<PktField>> & providers,
                          const std::map<PktField, std::string> & provider_expressions);

/// Transform codelets by reducing the number of incoming packet
/// fields if there are correlations between the packet fields
std::string sketch_preprocessor(const clang::TranslationUnitDecl * tu_decl);

/// Transform code blocks (or) equivalently, strongly connected
/// components into SKETCH specifications.
/// SKETCH specifications can then be fed to the SKETCH compiler to
/// synthesize configurations for atoms
/// (represented as sketches with holes).
std::string sketch_backend_transform(const clang::TranslationUnitDecl * tu_decl, const std::string t_atom_template);

/// Transfrom a Stmt representing a function body
/// into a SKETCH specification.
std::string create_sketch_spec(const clang::Stmt * function_body,
                               const std::string & spec_name);

/// Collect all state variable expressions recursively from a clang::Stmt
/// TODO: This is very similar to gen_var_list with a few minor differences.
/// All of these are symptoms that we need a recursive traversal mechansim
/// that is generic and spans all these AST traversal functions.
std::set<std::string> collect_state_vars(const clang::Stmt * function_body);

#endif  // SKETCH_BACKEND_H_
