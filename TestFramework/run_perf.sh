#!/bin/bash

rm -f $BASEDIR/run.log
touch $BASEDIR/run.log 

echo -e "Runtime\tCommand" >> $BASEDIR/run.log

echo 'STARTING EXECUTION'

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	bench_cmd=${cmds[2]}
	
	echo $bench_cmd

	$cd_cmd
	runtime=$(perf stat -r $REPEAT --log-fd 1 $bench_cmd | grep "seconds time elapsed" \
	       	| awk -F ' ' '{print $1}' \
		| sed -r 's/,/./g')

	echo -e "$runtime\t$bench_cmd" >> $BASEDIR/run.log
done < run.txt
