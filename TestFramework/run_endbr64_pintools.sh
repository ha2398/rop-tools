#!/bin/bash

MEM_LIMIT=2500000
PIN_PATH=$HOME/pin-3.7-gcc-linux
PINTOOLS_PATH=$PIN_PATH/source/tools/SimpleExamples
OUTPUT_FOLDER=endbr64_results
SOURCE_FOLDER=../Pintools/Endbr64

ROOT_DIR=${PWD}

function init() {
	rm -rf $OUTPUT_FOLDER
	mkdir $OUTPUT_FOLDER
	mkdir $OUTPUT_FOLDER/count_endbr64
	mkdir $OUTPUT_FOLDER/count_endbr64_pruned

	ulimit -m $MEM_LIMIT
	ulimit -v $MEM_LIMIT
}

function compile_pintools() {
	echo "### Compiling Pintools ###"
  	cp $SOURCE_FOLDER/count_endbr64.cpp $SOURCE_FOLDER/count_endbr64_pruned.cpp $PIN_PATH/source/tools/SimpleExamples
	cd $PINTOOLS_PATH
	make clean
	make dir obj-intel64/count_endbr64.so
	make dir obj-intel64/count_endbr64_pruned.so
	cd $ROOT_DIR
}

function run_pintools() {
	echo "### Running Pintools ###"
	COMPILE=0
	EXEC=1
	PIN=1
	PINTOOL="<pintool_placeholder>"
	source "run_endbr64.sh"
}

init
compile_pintools
run_pintools
