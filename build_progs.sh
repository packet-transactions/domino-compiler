#! /bin/bash
set -e
set -x
set -v
./clang_compile.sh canonicalize_if.cc clang_utility_functions.cc if_stmt_handler.cc
mv a.out canonicalize_if

./clang_compile.sh struct_to_local_vars.cc clang_utility_functions.cc
mv a.out struct_to_local_vars
