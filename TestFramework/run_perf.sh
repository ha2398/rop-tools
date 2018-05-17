#!/bin/bash

echo 'STARTING EXECUTION'

while read -r LINE; do
	perf stat -r 5 --log-fd 1 $LINE | grep time | awk -F ' ' '{print $1}' \
		| sed -r 's/,/./g'
done < cmds
