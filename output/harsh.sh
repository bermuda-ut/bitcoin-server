#!/bin/sh

while true
do
    echo Testing...
    ./test.sh $1 > /dev/null
    ps
    sleep 2
done

