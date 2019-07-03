#!/bin/bash

COMPLETE_FILE="$BASEDIR/complete.log"
PRUNED_FILE="$BASEDIR/pruned.log"
BENCHS_FILE="$BASEDIR/benchs.log"

BENCH_CMDS_FILE="run_bench.txt"
PRUNED_CMDS_FILE="run_pruned.txt"
COMPLETE_CMDS_FILE="run_complete.txt"
CMDS_FILE="cmds.txt"

PERF_OPTIONS=""

rm -rf $PRUNED_CMDS_FILE $COMPLETE_CMDS_FILE
#remove logs
rm -rf $COMPLETE_FILE $PRUNED_FILE $CMDS_FILE

while read -r LINE; do
	IFS="&&" read -ra cmds <<< "$LINE"
	cd_cmd=${cmds[0]}
	pin_cmd=${cmds[2]}
	bench_cmd="$(echo $LINE | awk -F"--" '{print $2}')"
	
	complete_pin_cmd="${pin_cmd//<pintool_placeholder>/count_endbr64}"
	pruned_pin_cmd="${pin_cmd//<pintool_placeholder>/count_endbr64_pruned}"

	# Grouping 1
	EVENTS="instructions:u,cpu-clock:u,r8488:u,r8489:u"
	echo "$cd_cmd && perf stat -r 5 $PERF_OPTIONS -o $COMPLETE_FILE --append" \
	     " -e $EVENTS $complete_pin_cmd" >> $BASEDIR/$COMPLETE_CMDS_FILE
	
	echo "$cd_cmd && perf stat -r 5 $PERF_OPTIONS -o $PRUNED_FILE --append" \
	     " -e $EVENTS $pruned_pin_cmd" >> $BASEDIR/$PRUNED_CMDS_FILE ;

	# Grouping 2
	EVENTS="instructions:u,cpu-clock:u,ra088:u,ra089:u"
	echo "$cd_cmd && perf stat -r 1 $PERF_OPTIONS -o $COMPLETE_FILE --append" \
	     " -e $EVENTS $complete_pin_cmd" >> $BASEDIR/$COMPLETE_CMDS_FILE ;

	echo "$cd_cmd && perf stat -r 1 $PERF_OPTIONS -o $PRUNED_FILE --append" \
	     " -e $EVENTS $pruned_pin_cmd" >> $BASEDIR/$PRUNED_CMDS_FILE ;
done < run.txt

if [[ $EXEC -eq 1 ]]; then
  echo 'STARTING EXECUTION' ;
  cat $PRUNED_CMDS_FILE $COMPLETE_CMDS_FILE > $CMDS_FILE ;
  rm -rf $PRUNED_CMDS_FILE $COMPLETE_CMDS_FILE ;
  parallel -v -j $JOBS < $CMDS_FILE 2>&1 | tee parallel.out ;
fi
