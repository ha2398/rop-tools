#!/bin/bash

PIN=1
PIN_PATH=$HOME/pin-3.4
PINTOOLS_PATH=$PIN_PATH/source/tools/SimpleExamples

ROOT_DIR=${PWD}

function init() {
	rm -rf overhead_outputs
	mkdir overhead_outputs
	mkdir overhead_outputs/complete
	mkdir overhead_outputs/pruned
}

function compile_pintools() {
	echo "### Compiling Pintools ###"
	cp ../Pintools/Overhead/complete.cpp ../Pintools/Overhead/pruned.cpp $PIN_PATH/source/tools/SimpleExamples
	cd $PINTOOLS_PATH
	make clean
	make dir obj-intel64/complete.so
	make dir obj-intel64/pruned.so
	cd $ROOT_DIR
}

function run_pintools() {
	echo "### Executing complete pintool ###"
	COMPILE=0 EXEC=1 PIN=1 RUNTIME=0 PINTOOL="complete" ./run.sh
	mv run.log complete.log
	
	COMPILE=0 EXEC=1 PIN=1 RUNTIME=0  PINTOOL="pruned" ./run.sh
	mv run.log pruned.log
	echo "### Executing pruned pintool ###"
}

init
compile_pintools
run_pintools
