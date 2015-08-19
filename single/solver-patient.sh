#!/bin/bash

# Problem number is the argument.
if [ $# -ne 1 ]; then
    echo "[solver-patient] Error: Problem number is needed." 1>&2
    exit 1
fi

while :
do
    echo "[solver-patient] Solver has just set up."
    ./solver --loop 500 --output a$1.txt p$1.txt
    if [ $? -eq 0 ]; then
        echo "[solver-patient] Solved."
        break
    fi
done

exit 0
