#ifndef PACKET_VARIABLE_CENSUS_H_
#define PACKET_VARIABLE_CENSUS_H_

#include <set>

#include "clang/AST/Decl.h"

/// Return the current set of identifiers
/// so that we can generate unique names afterwards
std::set<std::string> identifier_census(const clang::TranslationUnitDecl * decl);

#endif  // PACKET_VARIABLE_CENSUS_H_
