#!/bin/bash

PIN=1
PIN_PATH=$HOME/pin-3.4
PINTOOLS_PATH=$PIN_PATH/source/tools/SimpleExamples

ROOT_DIR=${PWD}

function compile_pintools() {
	cp ../Pintools/Overhead/complete.cpp ../Pintools/Overhead/pruned.cpp $PIN_PATH/source/tools/SimpleExamples
	cd $PINTOOLS_PATH
	make clean
	make obj-intel64/complete.so
	make obj-intel64/pruned.so
	cd $ROOT_DIR
}

function run_pintools() {
	COMPILE=0 EXEC=1 PIN=1 PINTOOL="complete" ./run.sh
	mv run.log complete.log
	
	COMPILE=0 EXEC=1 PIN=1 PINTOOL="pruned" ./run.sh
	mv run.log pruned.log
}

compile_pintools
