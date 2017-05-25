#!/bin/sh

while true
do
    echo "Testing.."
    ./a "$1" "$2" > /dev/null &
    pidA=`echo $!`

    ./b "$1" $2 > /dev/null &
    pidB=`echo $!`

    ./c "$1" $2 > /dev/null &
    pidC=`echo $!`

    ./d "$1" $2 > /dev/null &
    pidD=`echo $!`

    ./efgh "$1" $2 > /dev/null &
    pidE=`echo $!`

    ./i "$1" $2 > /dev/null &
    pidI=`echo $!`

    #./jk localhost "$1" > /dev/null &
    #pidJ=`echo $!`

    sleep 3

    ps
    kill -9 $pidA
    kill -9 $pidB
    kill -9 $pidC
    kill -9 $pidD
    kill -9 $pidE
    kill -9 $pidI
    #kill -9 $pidJ
    ps
done
