#!/bin/bash

OUT_FILE="$BASEDIR/run.log"
rm -f $BASEDIR/run.txt.temp $OUT_FILE

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	pin_cmd=${cmds[2]}
	bench_cmd="$(echo $LINE | awk -F"--" '{print $2}')"
	
	# Grouping 1
	EVENTS="instructions,cpu-clock,r8888"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $pin_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 2
	EVENTS="instructions,iTLB-load-misses,r8488,r8489"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 3
	EVENTS="instructions,r8888,r8889,ra088"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 4
	EVENTS="instructions,ra089"
	echo "$cd_cmd && perf stat -r $REPEAT -o $OUT_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp
done < run.txt

mv run.txt.temp run.txt
parallel -j $JOBS < run.txt > parallel.out
