#! /bin/bash
# If convert
./if_converter $1 -- 2> /tmp/dep.dot

echo "Original: "
cat $1
echo "************"
echo "Dep graph: "
cat /tmp/dep.dot | dot -T png | display
