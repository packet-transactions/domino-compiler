#ifndef PACKET_VARIABLE_CENSUS_H_
#define PACKET_VARIABLE_CENSUS_H_

#include <set>
#include "clang/AST/AST.h"

/// Return the current set of packet variables
/// so that we can generate unique names afterwards
std::set<std::string> packet_variable_census(const clang::TranslationUnitDecl * decl);

#endif  // PACKET_VARIABLE_CENSUS_H_
