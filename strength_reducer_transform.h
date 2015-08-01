#ifndef STRENGTH_REDUCER_TRANSFORM_H_
#define STRENGTH_REDUCER_TRANSFORM_H_

#include <iostream>
#include <string>
#include "clang/AST/AST.h"
#include "clang_utility_functions.h"

/// Function that is passed to SinglePass
std::string strength_reducer_transform(const clang::TranslationUnitDecl * tu_decl);

/// Helper carrying out actual work
std::string strength_reduce_helper(const clang::FunctionDecl * function_decl);

#endif  // STRENGTH_REDUCER_TRANSFORM_H_
