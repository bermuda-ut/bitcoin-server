#!/bin/sh

./client-p localhost 1234 &
./client-pp localhost 1234 &
./client-s1 localhost 1234 &
./client-s2 localhost 1234 &
./client-s3 localhost 1234 &

for i in `seq 1 10`;
do
    ./client-p localhost 1234 &
done 
