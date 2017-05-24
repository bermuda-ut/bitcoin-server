#!/bin/sh

./client-p localhost "$1" &
./client-s1 localhost "$1" &
./client-s2 localhost "$1" &
./client-s3 localhost "$1" &
./client-sl localhost "$1" &

for i in `seq 1 1000`;
do
    ./client-p localhost "$1" &
done 
sleep 1

for i in `seq 1 1000`;
do
    ./client-s1 localhost "$1" &
done 
sleep 1

for i in `seq 1 1000`;
do
    ./client-s2 localhost "$1" &
done 
sleep 1

for i in `seq 1 1000`;
do
    ./client-s3 localhost "$1" &
done 
sleep 1
