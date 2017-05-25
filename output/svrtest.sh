#!/bin/sh

while true
do
    echo "Testing.."
    ./a localhost "$1" > /dev/null &
    pidA=`echo $!`

    ./b localhost "$1" > /dev/null &
    pidB=`echo $!`

    ./c localhost "$1" > /dev/null &
    pidC=`echo $!`

    ./d localhost "$1" > /dev/null &
    pidD=`echo $!`

    ./efgh localhost "$1" > /dev/null &
    pidE=`echo $!`

    ./i localhost "$1" > /dev/null &
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
