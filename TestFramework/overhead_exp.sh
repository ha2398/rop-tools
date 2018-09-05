#!/bin/bash

COMPLETE_FILE="$BASEDIR/complete.log"
PRUNED_FILE="$BASEDIR/pruned.log"
BENCHS_FILE="$BASEDIR/benchs.log"

BENCH_CMDS_FILE="run_bench.txt"
BENCH_CMDS_FILE2="run_bench2.txt"
PRUNED_CMDS_FILE="run_pruned.txt"
COMPLETE_CMDS_FILE="run_complete.txt"
CMDS_FILE="cmds.txt"

rm -rf $BENCH_CMDS_FILE $BENCH_CMDS_FILE2 $PRUNED_CMDS_FILE $COMPLETE_CMDS_FILE
#remove logs
rm -rf $COMPLETE_FILE $PRUNED_FILE $BENCHS_FILE $CMDS_FILE

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	pin_cmd=${cmds[2]}
	bench_cmd="$(echo $LINE | awk -F"--" '{print $2}')"
	echo $LINE
	
	complete_pin_cmd="${pin_cmd//<pintool_placeholder>/complete}"
	pruned_pin_cmd="${pin_cmd//<pintool_placeholder>/pruned}"

	# Grouping 1
	EVENTS="instructions,cpu-clock"
	echo "$cd_cmd && perf stat -r $REPEAT -o $COMPLETE_FILE --append" \
	     " -e $EVENTS $complete_pin_cmd" >> $BASEDIR/$COMPLETE_CMDS_FILE
	
	echo "$cd_cmd && perf stat -r $REPEAT -o $PRUNED_FILE --append" \
	     " -e $EVENTS $pruned_pin_cmd" >> $BASEDIR/$PRUNED_CMDS_FILE

	# Grouping 5
	EVENTS="instructions,cpu-clock,r8888,r8889"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append" \
	     " -e $EVENTS $bench_cmd" >> $BASEDIR/$BENCH_CMDS_FILE2

	# Grouping 2
	EVENTS="instructions,iTLB-load-misses,r8488,r8489"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append" \
	     " -e $EVENTS $bench_cmd" >> $BASEDIR/$BENCH_CMDS_FILE

	# Grouping 3
	EVENTS="instructions,r8888,r8889,ra088"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append" \
	     " -e $EVENTS $bench_cmd" >> $BASEDIR/$BENCH_CMDS_FILE

	# Grouping 4
	EVENTS="instructions,ra089"
	echo "$cd_cmd && perf stat -r $REPEAT -o $BENCHS_FILE --append" \
	     " -e $EVENTS $bench_cmd" >> $BASEDIR/$BENCH_CMDS_FILE
done < run.txt

cat $BENCH_CMDS_FILE2 $PRUNED_CMDS_FILE $COMPLETE_CMDS_FILE $BENCH_CMDS_FILE > $CMDS_FILE

if [[ $EXEC -eq 1 ]]; then
  echo 'STARTING EXECUTION' ;
  parallel -j $JOBS < $CMDS_FILE > parallel.out ;
fi
