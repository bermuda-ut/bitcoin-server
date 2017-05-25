#!/bin/sh

./client-p localhost "$1" &
./client-s1 localhost "$1" &
./client-s2 localhost "$1" &
./client-s3 localhost "$1" &
./client-sl localhost "$1" &
pkill client-sl

for i in `seq 1 80`;
do
    ./client-p localhost "$1" &
done 
sleep 2
pkill client-p

for i in `seq 1 80`;
do
    ./client-s1 localhost "$1" &
done 
sleep 2
pkill client-s1

for i in `seq 1 80`;
do
    ./client-s2 localhost "$1" &
done 
sleep 2
pkill client-s2

for i in `seq 1 80`;
do
    ./client-s3 localhost "$1" &
done 
sleep 2
pkill client-s3
