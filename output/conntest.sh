#!/bin/sh

for i in `seq 1 100`;
do
    netcat -C localhost $1 &
done

sleep 1234
