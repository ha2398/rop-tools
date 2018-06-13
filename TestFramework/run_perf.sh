#!/bin/bash

OUT_FILE="$BASEDIR/run.log"
rm -f $BASEDIR/run.txt.temp $OUT_FILE

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	bench_cmd=${cmds[2]}
	
	# Grouping 1
	EVENTS="instructions,cpu-clock,iTLB-load-misses,r8488"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 2
	EVENTS="instructions,r8489,r8888,r8889"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 3
	EVENTS="instructions,ra088,ra089"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp
done < run.txt

mv run.txt.temp run.txt

parallel -j $JOBS < run.txt > parallel.out
