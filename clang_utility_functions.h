#ifndef CLANG_UTILITY_FUNCTIONS_H_
#define CLANG_UTILITY_FUNCTIONS_H_

#include <set>
#include <string>
#include <map>

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"

/// Enum class to represent Variable type (packet (p.x) or state (x), or both)
enum class VariableType {PACKET, STATE, FUNCTION_PARAMETER};

/// Map from VariableType to bool,
/// denoting whether a variable should be selected or not
typedef std::map<VariableType, bool> VariableTypeSelector;

/// General puprose printer for clang stmts
/// Everything executable subclasses from clang::Stmt, including clang::Expr
std::string clang_stmt_printer(const clang::Stmt * stmt);

/// Print name of a value declaration (http://clang.llvm.org/doxygen/classclang_1_1ValueDecl.html#details)
/// We use it to print:
/// 1. Field names within a packet structure.
/// 2. Packet parameter names passed to packet functions.
/// 3. State variable names.
std::string clang_value_decl_printer(const clang::ValueDecl * value_decl);

/// Print all kinds of clang declarations
/// This is best used when we want to pass through certain statements unchanged.
/// It prints the entire declaration (along with the definition if it accompanies the declaration).
std::string clang_decl_printer(const clang::Decl * decl);

/// Is this a packet function: does it have struct Packet as an argument
bool is_packet_func(const clang::FunctionDecl * func_decl);

/// Return the current set of identifiers
/// so that we can generate unique names afterwards
std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl,
                                        const VariableTypeSelector & var_selector =
                                        {{VariableType::PACKET, true}, {VariableType::FUNCTION_PARAMETER, true}, {VariableType::STATE, true}});

/// Determine all variables (either packet or state) used within a clang::Stmt,
std::set<std::string> gen_var_list(const clang::Stmt * stmt,
                                   const VariableTypeSelector & var_selector =
                                   {{VariableType::PACKET, true}, {VariableType::STATE, true}}
                                   );

/// Generate scalar function declarations,
/// including function definitions if provided
std::string generate_scalar_func_def(const clang::FunctionDecl * func_decl);

/// List out all packet fields in a translation unit,
/// by first calling identifier_census and then serializing the result
std::string gen_pkt_fields(const clang::TranslationUnitDecl * tu_decl);

#endif  // CLANG_UTILITY_FUNCTIONS_H_
