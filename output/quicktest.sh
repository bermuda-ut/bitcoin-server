#!/bin/sh

while true
do
    echo "Testing.."
    ./a localhost "$1" > /dev/null &
    ./b localhost "$1" > /dev/null &
    ./c localhost "$1" > /dev/null &
    ./d localhost "$1" > /dev/null &
    ./efgh localhost "$1" > /dev/null &
    ./i localhost "$1" > /dev/null &
    sleep 3
    ps
done
