#! /bin/bash
# If convert
./if_converter $1 -- > /tmp/if_converted.c

# Strength reduction
./strength_reducer /tmp/if_converted.c -- > /tmp/strength_reduced.c

# Iterate expr_flattener till a fixed point
cp /tmp/strength_reduced.c /tmp/flat.c
while [ 1 -lt 2 ] ;
do
  ./expr_flattener /tmp/flat.c -- > /tmp/flat_more.c
  if diff /tmp/flat.c /tmp/flat_more.c;
  then
    mv /tmp/flat_more.c /tmp/flat.c
    break;
  else
    mv /tmp/flat_more.c /tmp/flat.c
    continue;
  fi
done

# Propagate expressions to their uses
./expr_propagater /tmp/flat.c -- > /tmp/prop.c

# Run stateful_flanks on /tmp/prop.c to add prologue and epilpogue for state vars
./stateful_flanks /tmp/prop.c -- > /tmp/stateful_flanks.c

# Run ssa on /tmp/stateful_flanks.c to generate Stateful Static Single Assignment
./ssa /tmp/stateful_flanks.c -- > /tmp/ssa.c

# Run dep_graph on /tmp/ssa.c to generate final dependency graph after condensation
./dep_graph /tmp/ssa.c -- 2> /tmp/dep.dot

echo "Original: "
cat $1
echo "************"
echo "Dep graph: "
cat /tmp/dep.dot | dot -T png | display
