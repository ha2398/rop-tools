#!/bin/bash

MEM_LIMIT=2500000
PIN_PATH=$HOME/pin-3.4
PINTOOLS_PATH=$PIN_PATH/source/tools/SimpleExamples

OVERHEAD_PINTOOLS_PATH=$HOME/rop-tools/Pintools/Overhead/
ROOT_DIR=${PWD}

CPU2006_FLAGS="--tune=base --size=ref --iterations=1 --noreportable"
COMPILE_FLAGS="--tune=base --action=build"
CONFIGS_FOLDER=$ROOT_DIR/configs

COMPLETE_LOG="$ROOT_DIR/complete.log"
PRUNED_LOG="$ROOT_DIR/pruned.log"
BENCH_LOG="$ROOT_DIR/benchs.log"

CMDS_PERF1="cmds_perf_e1.txt"
CMDS_PERF2="cmds_perf_e2.txt"
CMDS_PERF3="cmds_perf_e3.txt"
CMDS_PIN_COMPLETE="cmds_pin_complete.txt"
CMDS_PIN_PRUNED="cmds_pin_pruned.txt"

PARALLEL_LOG="parallel.log"

declare -a benchs=("400.perlbench" "401.bzip2" "403.gcc" "429.mcf" "445.gobmk" "456.hmmer" "458.sjeng" "462.libquantum" "464.h264ref" "471.omnetpp" "473.astar" "483.xalancbmk" "999.specrand" "410.bwaves" "416.gamess" "433.milc" "434.zeusmp" "435.gromacs" "436.cactusADM" "437.leslie3d" "444.namd" "447.dealII" "450.soplex" "453.povray" "454.calculix" "459.GemsFDTD" "465.tonto" "470.lbm" "481.wrf" "482.sphinx3")

function init() {
	rm -rf overhead_outputs
	rm -rf configs/*.cfg.*
    rm -rf $COMPLETE_LOG $PRUNED_LOG $BENCH_LOG $PARALLEL_LOG
	rm -rf $CMDS_PERF1 $CMDS_PERF2 $CMDS_PERF3 $CMDS_PIN_COMPLETE $CMDS_PIN_PRUNED
	mkdir overhead_outputs
	mkdir overhead_outputs/complete
	mkdir overhead_outputs/pruned
	
	ulimit -m $MEM_LIMIT
	ulimit -v $MEM_LIMIT
}

function compile_pintools() {
	cp $OVERHEAD_PINTOOLS_PATH/complete.cpp $OVERHEAD_PINTOOLS_PATH/pruned.cpp $PIN_PATH/source/tools/SimpleExamples

	cd $PINTOOLS_PATH
	make clean
	make dir obj-intel64/complete.so
	make dir obj-intel64/pruned.so
	cd $ROOT_DIR
}

function run_cpu2006() {
	source ../shrc

	for bench in "${benchs[@]}"
	do	
		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/complete_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/complete_$bench.cfg $bench" >> $CMDS_PIN_COMPLETE
		
		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/pruned_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/pruned_$bench.cfg $bench" >> $CMDS_PIN_PRUNED

		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/perf_e1.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/perf_e1.cfg $bench" >> $CMDS_PERF1

		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/perf_e2.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/perf_e2.cfg $bench" >> $CMDS_PERF2

		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/perf_e3.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/perf_e3.cfg $bench" >> $CMDS_PERF3
	done

	[[ -n $JOBS ]] || JOBS=4 ;
	
	parallel -j $JOBS < $CMDS_PERF1 >> $PARALLEL_LOG
	parallel -j $JOBS < $CMDS_PERF2 >> $PARALLEL_LOG
	parallel -j $JOBS < $CMDS_PERF3 >> $PARALLEL_LOG
	parallel -j $JOBS < $CMDS_PIN_PRUNED >> $PARALLEL_LOG
	parallel -j $JOBS < $CMDS_PIN_COMPLETE >> $PARALLEL_LOG
}

init
compile_pintools
run_cpu2006
