#!/bin/sh

./client-p localhost 1234 &
./client-s1 localhost 1234 &
./client-s2 localhost 1234 &
./client-s3 localhost 1234 &

./client-sl localhost 1234 &

for i in `seq 1 200`;
do
    ./client-s1 localhost 1234 &
done 

for i in `seq 1 200`;
do
    ./client-sl localhost 1234 &
done 

for i in `seq 1 200`;
do
    ./client-s2 localhost 1234 &
done 

for i in `seq 1 200`;
do
    ./client-s3 localhost 1234 &
done 
