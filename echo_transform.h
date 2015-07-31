#ifndef ECHO_TRANSFORM_H_
#define ECHO_TRANSFORM_H_

#include "clang/AST/Decl.h"
#include "clang_utility_functions.h"

// Echo Transform takes in a TranslationUnitDecl and
// prints it out as such with no mods.
auto echo_transform(const clang::TranslationUnitDecl * tu_decl) { return clang_decl_printer(tu_decl); }

#endif // ECHO_TRANSFORM_H_
