#!/bin/sh

for i in `seq 1 10`;
do
    echo "Testing.."
    ./efgh localhost "$1" > /dev/null &
done
