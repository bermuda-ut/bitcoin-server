#!/bin/sh

while true
do
    echo Testing...
    ./test.sh $1 > /dev/null
    sleep 20
    ps
done

