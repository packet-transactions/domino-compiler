#ifndef PISA_ATOM_H_
#define PISA_ATOM_H_

#include <set>
#include <string>
#include <map>
#include <field_container.h> // for banzai's FieldContainer::FieldType

#include "clang/AST/Stmt.h"

/// Store PISA Atoms as strings that represent C++ code containing
/// 1. Atom name
/// 2. Atom function signature
/// 3. Atom function body.
/// 4. Atom state initializers
class PISAAtom {
 public:
  typedef int ScalarValue;
  typedef uint64_t ArraySize;
  typedef std::string VariableName;
  typedef std::map<VariableName, ScalarValue> ScalarInitializer;
  typedef std::map<VariableName, std::pair<ArraySize, ScalarValue>> ArrayInitializer;
  typedef std::set<VariableName> VariableSet;

  /// Packet identifier when generating banzai code
  static const std::string PACKET_IDENTIFIER;

  /// State scalar identifier when generating banzai code
  static const std::string STATE_SCALAR_IDENTIFIER;

  /// State array identifier when generating banzai code
  static const std::string STATE_ARRAY_IDENTIFIER;

  /// Construct a PISAAtom from a clang::Stmt
  /// by calling rewrite_into_banzai_ops to generate the body,
  /// prepending a function signature,
  /// and then adding state initializers
  PISAAtom(const clang::Stmt * stmt,
             const std::string & t_atom_name,
             const ScalarInitializer & scalar_initializer,
             const ArrayInitializer  & array_initializer);

  /// Retrieve atom declaration/definition as string
  auto get_def() const { return atom_definition_; }

  /// Get name of the atom
  auto get_name() const { return name_; }

 private:
  /// Get a scalar initializer string, i.e. identify all scalar stateful
  /// variables within stmt / function_body_ using gen_var_list(). Then get their
  /// initial values using scalar_initializer (provided in constructor)
  /// and create a FieldContainer to hold everything
  std::string scalar_init_string(const std::set<std::string> & state_scalars_used,
                                 const ScalarInitializer & scalar_initializer) const;

  /// Same as scalar_init_string, but for stateful arrays
  std::string array_init_string(const std::set<std::string> & array_vars_used,
                                const ArrayInitializer & array_initializer) const;

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

  /// State scalars used by function_body_/stmt
  std::set<std::string> state_scalars_used_;

  /// State arrays used by function_body_/stmt
  std::set<std::string> state_arrays_used_;

  /// Atom definition (combines atom declaration with definition)
  /// i.e. the function representing the atom and the state initializer
  /// using the function_definition_ and name_ that is provided
  std::string atom_definition_;
};

#endif  // PISA_ATOM_H_
