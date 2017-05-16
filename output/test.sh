#!/bin/sh

./client-p localhost 1234 &
./client-s1 localhost 1234 &
./client-s2 localhost 1234 &
./client-s3 localhost 1234 &

for i in `seq 1 100`;
do
    ./client-s1 localhost 1234 &
    ./client-s2 localhost 1234 &
    ./client-s3 localhost 1234 &
done 
