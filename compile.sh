#! /bin/bash
# If convert
./if_conversion_prog $1 -- > /tmp/if_converted.c

# Strength reduction
./strength_reductor /tmp/if_converted.c -- > /tmp/strength_reduced.c

# Iterate expr_flattener_prog till a fixed point
cp /tmp/strength_reduced.c /tmp/flat.c
while [ 1 -lt 2 ] ;
do
  ./expr_flattener_prog /tmp/flat.c -- > /tmp/flat_more.c
  if diff /tmp/flat.c /tmp/flat_more.c;
  then
    mv /tmp/flat_more.c /tmp/flat.c
    break;
  else
    mv /tmp/flat_more.c /tmp/flat.c
    continue;
  fi
done

# Run partitioning_prog on /tmp/flat.c
./partitioning_prog /tmp/flat.c -- > /tmp/partition.txt

echo "Original: "
cat $1
echo "************"
echo "Partitioned: "
cat /tmp/partition.txt
