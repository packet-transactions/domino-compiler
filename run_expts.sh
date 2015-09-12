#! /bin/bash
for domino_file in `cat $1`
do
  for atom_template in `cat $2`
  do
    echo $domino_file
    echo $atom_template
    ATOM_TEMPLATE=$atom_template domino $domino_file int_type_checker,desugar_comp_asgn,if_converter,algebra_simplify,array_validator,stateful_flanks,ssa,expr_propagater,expr_flattener,partitioning,sketch_backend 2>&1 | grep "with random seed"
    echo ""
    echo ""
  done
done

