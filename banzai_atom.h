#ifndef BANZAI_ATOM_H_
#define BANZAI_ATOM_H_

#include <set>
#include <string>
#include <map>
#include <field_container.h> // for banzai's FieldContainer::FieldType

#include "clang/AST/Stmt.h"

/// Store Banzai Atoms as strings that represent C++ code containing
/// 1. Atom name
/// 2. Atom function signature
/// 3. Atom function body.
/// 4. Atom state initializers
class BanzaiAtom {
 public:
  /// Packet identifier when generating banzai code
  static const std::string PACKET_IDENTIFIER;

  /// State identifier when generating banzai code
  static const std::string STATE_IDENTIFIER;

  /// Construct a BanzaiAtom from a clang::Stmt
  /// by calling rewrite_into_banzai_ops to generate the body,
  /// prepending a function signature,
  /// and then adding state initializers
  BanzaiAtom(const clang::Stmt * stmt,
             const std::string & t_atom_name,
             const std::map<FieldContainer::FieldName, FieldContainer::FieldType> & state_initializers);

  /// Retrieve atom declaration/definition as string
  auto get_def() const { return atom_definition_; }

  /// Get name of the atom
  auto get_name() const { return name_; }

 private:
  /// Get a state initializer string, i.e. identify all state variables
  /// within stmt / function_body_ using gen_var_list(). Then get their
  /// initial values using state_initializers (provided in constructor)
  /// and create a FieldContainer to hold everything
  std::string state_init_string(const std::set<std::string> & state_vars_used,
                                const std::map<FieldContainer::FieldName, FieldContainer::FieldType> & state_initializers) const;

  /// Transform a given clang::Stmt into banzai ops, by replacing
  /// field = expr with state("field") or pkt("field") = expr as the 
  /// case may be.
  std::string rewrite_into_banzai_ops(const clang::Stmt * stmt) const;

  /// The name of the atom
  std::string name_;

  /// Just the function body of the atom
  std::string function_body_;

  /// Atom function name + signature + body as a string
  std::string function_definition_;

  /// State variables used by function_body_/stmt
  std::set<std::string> state_vars_used_;

  /// Atom definition (combines atom declaration with definition)
  /// i.e. the function representing the atom and the state initializer
  /// using the function_definition_ and name_ that is provided
  std::string atom_definition_;
};

#endif  // BANZAI_ATOM_H_
