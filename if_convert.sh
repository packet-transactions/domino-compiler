#! /bin/bash
set -e
set -x
set -v

# Run struct_to_local_vars on file first
./struct_to_local_vars $1 -- > repl.yaml
clang-apply-replacements .

# Iterate till a fixed point
while [ 1 -lt 2 ] ;
do
  cp $1 old.cc
  ./if_conversion_prog $1 -- > repl.yaml
  clang-apply-replacements .
  cp $1 new.cc
  if diff old.cc new.cc;
  then
    break;
  else
    continue;
  fi
done

rm repl.yaml old.cc new.cc
clang-format-3.5 -i $1
