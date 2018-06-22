#!/bin/bash

COMPLETE_FILE="$BASEDIR/complete.log"
PRUNED_FILE="$BASEDIR/pruned.log"
BENCHS_FILE="$BASEDIR/benchs.log"

rm -f $BASEDIR/run.txt.temp $COMPLETE_FILE $PRUNED_FILE $BENCHS_FILE

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	pin_cmd=${cmds[2]}
	bench_cmd="$(echo $LINE | awk -F"--" '{print $2}')"
	
	complete_pin_cmd="${pin_cmd//<pintool_placeholder>/complete}"
	pruned_pin_cmd="${pin_cmd//<pintool_placeholder>/pruned}"

	# Grouping 1
	EVENTS="instructions,cpu-clock,r8888"
	echo "$cd_cmd && perf stat -r $REPEAT -o $COMPLETE_FILE --append \
	      -e $EVENTS $complete_pin_cmd" >> $BASEDIR/run.txt.temp
	
	echo "$cd_cmd && perf stat -r $REPEAT -o $PRUNED_FILE --append \
	      -e $EVENTS $pruned_pin_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 2
	EVENTS="instructions,iTLB-load-misses,r8488,r8489"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 3
	EVENTS="instructions,r8888,r8889,ra088"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp

	# Grouping 4
	EVENTS="instructions,ra089"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append \
	      -e $EVENTS $bench_cmd" >> $BASEDIR/run.txt.temp
done < run.txt

mv run.txt.temp run.txt
parallel -j $JOBS < run.txt > parallel.out
