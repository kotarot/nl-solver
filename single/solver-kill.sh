#!/bin/bash -x

# Kill `solver` process

if [ $OS = "Windows_NT" ]
then
    PIDS=(`ps aux | grep solver | grep -v grep | awk '{ print $1 }'`)
else
    PIDS=(`ps aux | grep solver | grep -v grep | awk '{ print $2; }'`)
fi

for pid in ${PIDS[*]}
do
kill -9 ${pid}
done


# Kill `python (top.py)` process

if [ $OS = "Windows_NT" ]
then
    PIDS=(`ps aux | grep python | grep -v grep | awk '{ print $1 }'`)
else
    PIDS=(`ps aux | grep python | grep -v grep | awk '{ print $2; }'`)
fi

for pid in ${PIDS[*]}
do
kill -9 ${pid}
done
