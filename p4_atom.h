#ifndef P4_ATOM_H_
#define P4_ATOM_H_

#include <set>
#include <string>
#include <map>
#include <field_container.h> // for p4's FieldContainer::FieldType

#include "clang/AST/Stmt.h"

/// Store P4 Atoms as strings that represent C++ code containing
/// 1. Atom name
/// 2. Atom function signature
/// 3. Atom function body.
/// 4. Atom state initializers
class P4Atom {
 public:
  typedef int ScalarValue;
  typedef uint64_t ArraySize;
  typedef std::string VariableName;
  typedef std::map<VariableName, ScalarValue> ScalarInitializer;
  typedef std::map<VariableName, std::pair<ArraySize, ScalarValue>> ArrayInitializer;
  typedef std::set<VariableName> VariableSet;

  /// Packet identifier when generating p4 code
  static const std::string PACKET_IDENTIFIER;

  /// State scalar identifier when generating p4 code
  static const std::string STATE_SCALAR_IDENTIFIER;

  /// State array identifier when generating p4 code
  static const std::string STATE_ARRAY_IDENTIFIER;

  /// Construct a P4Atom from a clang::Stmt
  /// by calling rewrite_into_p4_ops to generate the body,
  /// prepending a function signature,
  /// and then adding state initializers
  P4Atom(const clang::Stmt * stmt,
         const std::string & t_atom_name);

  /// Retrieve atom declaration/definition as string
  auto get_def() const { return atom_definition_; }

  /// Get name of the atom
  auto get_name() const { return name_; }

 private:
  /// Transform a given clang::Stmt into p4 ops, by replacing
  /// field = expr with state("field") or pkt("field") = expr as the 
  /// case may be.
  std::string rewrite_into_p4_ops(const clang::Stmt * stmt);

  /// Field list string (for hash functions in P4)
  std::string field_list_str_;

  /// The name of the atom
  std::string name_;

  /// Just the function body of the atom
  std::string function_body_;

  /// Atom definition (combines atom declaration with definition)
  /// i.e. the function representing the atom and the state initializer
  /// using the function_definition_ and name_ that is provided
  std::string atom_definition_;
};

#endif  // P4_ATOM_H_
