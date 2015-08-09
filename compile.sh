#! /bin/bash
# If convert
./if_converter $1 -- > /tmp/if_converted.c

# Run stateful_flanks on /tmp/prop.c to add prologue and epilpogue for state vars
./stateful_flanks /tmp/if_converted.c -- > /tmp/stateful_flanks.c

# Run ssa on /tmp/stateful_flanks.c to generate Stateful Static Single Assignment
./ssa /tmp/stateful_flanks.c -- > /tmp/ssa.c

# Run dep_graph on /tmp/ssa.c to generate final dependency graph after condensation
./dep_graph /tmp/ssa.c -- 2> /tmp/dep.dot

echo "Original: "
cat $1
echo "************"
echo "Dep graph: "
cat /tmp/dep.dot | dot -T png | display
