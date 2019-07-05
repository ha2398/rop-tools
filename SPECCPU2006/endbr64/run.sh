#!/bin/bash

MEM_LIMIT=2500000
PIN_PATH=$HOME/pin-3.7-gcc-linux
PINTOOLS_PATH=$PIN_PATH/source/tools/SimpleExamples

ENDBR64_PINTOOLS_PATH=$HOME/rop-tools/Pintools/Endbr64/
ROOT_DIR=${PWD}

CPU2006_FLAGS="--tune=base --size=ref --iterations=1 --noreportable"
COMPILE_FLAGS="--tune=base --action=build"
CONFIGS_FOLDER=$ROOT_DIR/configs

COMPLETE_LOG="$ROOT_DIR/complete.log"
PRUNED_LOG="$ROOT_DIR/pruned.log"

CMDS_PIN_COMPLETE="cmds_pin_complete.txt"
CMDS_PIN_PRUNED="cmds_pin_pruned.txt"
CMDS_FILE="cmds.txt"

PARALLEL_LOG="parallel.log"

declare -a benchs=("400.perlbench" "401.bzip2" "403.gcc" "429.mcf" "445.gobmk" "456.hmmer" "458.sjeng" "462.libquantum" "464.h264ref" "471.omnetpp" "473.astar" "483.xalancbmk" "999.specrand" "410.bwaves" "416.gamess" "433.milc" "434.zeusmp" "435.gromacs" "436.cactusADM" "437.leslie3d" "444.namd" "447.dealII" "450.soplex" "453.povray" "454.calculix" "459.GemsFDTD" "465.tonto" "470.lbm" "481.wrf" "482.sphinx3")

function init() {
	rm -rf endbr64_outputs
	rm -rf configs/*.cfg.*
    rm -rf $COMPLETE_LOG $PRUNED_LOG $PARALLEL_LOG
	rm -rf $CMDS_PIN_COMPLETE $CMDS_PIN_PRUNED
	mkdir endbr64_outputs
	mkdir endbr64_outputs/count_endbr64
	mkdir endbr64_outputs/count_endbr64_pruned
	
	ulimit -m $MEM_LIMIT
	ulimit -v $MEM_LIMIT
}

function compile_pintools() {
	cp $ENDBR64_PINTOOLS_PATH/count_endbr64.cpp $ENDBR64_PINTOOLS_PATH/count_endbr64_pruned.cpp $PIN_PATH/source/tools/SimpleExamples

	cd $PINTOOLS_PATH
	make clean
	make dir obj-intel64/count_endbr64.so
	make dir obj-intel64/count_endbr64_pruned.so
	cd $ROOT_DIR
}

function run_cpu2006() {
	source ../shrc

	for bench in "${benchs[@]}"
	do	
		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/complete1_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/complete1_$bench.cfg $bench" >> $CMDS_PIN_COMPLETE
		
		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/pruned1_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/pruned1_$bench.cfg $bench" >> $CMDS_PIN_PRUNED

		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/complete2_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/complete2_$bench.cfg $bench" >> $CMDS_PIN_COMPLETE
		
		runspec $COMPILE_FLAGS --config=$CONFIGS_FOLDER/pruned2_$bench.cfg $bench
		echo "runspec $CPU2006_FLAGS --config=$CONFIGS_FOLDER/pruned2_$bench.cfg $bench" >> $CMDS_PIN_PRUNED
	done

	[[ -n $JOBS ]] || JOBS=4 ;

	cat $CMDS_PIN_PRUNED $CMDS_PIN_COMPLETE > $CMDS_FILE
	
	parallel -v -j $JOBS < $CMDS_FILE >> $PARALLEL_LOG
}

init
compile_pintools
run_cpu2006
