#!/bin/bash -x

for i in {1..9}; do
    ../conmgr/client/adccli --URL='http://192.168.11.201:8888/' --output="NL_Q0$i.txt" get-q $i
done
for i in {10..31}; do
    ../conmgr/client/adccli --URL='http://192.168.11.201:8888/' --output="NL_Q$i.txt" get-q $i
done

chmod 644 NL_Q*.txt
