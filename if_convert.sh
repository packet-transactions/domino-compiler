#! /bin/bash
set -e
set -x
set -v

# Run struct_to_local_vars on file first
./struct_to_local_vars $1 -- > repl.yaml
clang-apply-replacements .

./if_conversion_prog $1 -- | clang-format-3.5
