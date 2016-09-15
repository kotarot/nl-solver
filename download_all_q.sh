#!/bin/bash -x

for i in {1..9}; do
    ../conmgr/client/adccli --URL='http://das-adc.appspot.com/' --output="NL_Q0$i.txt" get-q $i
done
for i in {10..35}; do
    ../conmgr/client/adccli --URL='http://das-adc.appspot.com/' --output="NL_Q$i.txt" get-q $i
done

chmod 644 NL_Q*.txt
