#!/bin/bash

rm -f $BASEDIR/run.txt.temp $BASEDIR/run.log

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	bench_cmd=${cmds[2]}
	
	echo "$cd_cmd && 3> /dev/stdout perf stat -r $REPEAT --log-fd 3 \
	       	$bench_cmd" >> $BASEDIR/run.txt.temp
done < run.txt

mv run.txt.temp run.txt

parallel -j $JOBS < run.txt > parallel.out
cat parallel.out | grep "Performance counter stats\|seconds time elapsed" | sed -r 's/,/./g' > run.log
rm -f parallel.out
