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

# Run partitioner on /tmp/prop.c
./partitioner /tmp/prop.c -- > /tmp/partition.txt

echo "Original: "
cat $1
echo "************"
echo "Partitioned: "
cat /tmp/partition.txt
