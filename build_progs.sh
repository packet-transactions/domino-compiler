#! /bin/bash
set -e
set -x
set -v
./clang_compile.sh if_conversion_prog.cc clang_utility_functions.cc if_conversion_handler.cc
mv a.out if_conversion_prog

./clang_compile.sh struct_to_local_vars.cc clang_utility_functions.cc
mv a.out struct_to_local_vars

./clang_compile.sh partitioning_prog.cc partitioning_handler.cc clang_utility_functions.cc
mv a.out partitioning_prog
