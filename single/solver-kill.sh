#!/bin/bash -x

PIDS=(`ps aux | grep solver | grep -v grep | awk '{ print $2; }'`)
for pid in ${PIDS[*]}
do
kill -9 ${pid}
done
