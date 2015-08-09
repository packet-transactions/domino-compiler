#! /bin/bash
# If convert
./if_converter $1 -- > /tmp/if_converted.c

# Run dep_graph on /tmp/ssa.c to generate final dependency graph after condensation
./dep_graph /tmp/if_converted.c -- 2> /tmp/dep.dot

echo "Original: "
cat $1
echo "************"
echo "Dep graph: "
cat /tmp/dep.dot | dot -T png | display
