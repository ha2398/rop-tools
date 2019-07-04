#!/bin/bash

declare -a benchs=("400.perlbench" "401.bzip2" "403.gcc" "429.mcf" "445.gobmk" "456.hmmer" "458.sjeng" "462.libquantum" "464.h264ref" "471.omnetpp" "473.astar" "483.xalancbmk" "999.specrand" "410.bwaves" "416.gamess" "433.milc" "434.zeusmp" "435.gromacs" "436.cactusADM" "437.leslie3d" "444.namd" "447.dealII" "450.soplex" "453.povray" "454.calculix" "459.GemsFDTD" "465.tonto" "470.lbm" "481.wrf" "482.sphinx3" "998.specrand")

ROOT_DIR="/usr/cpu2006/endbr64"
OUTPUT_DIR="$ROOT_DIR/endbr64_outputs"
COMPLETE_LOG="$ROOT_DIR/complete.log"
PRUNED_LOG="$ROOT_DIR/pruned.log"
BENCH_LOG="$ROOT_DIR/benchs.log"

for bench in "${benchs[@]}"
do
	# Complete pintool command (Grouping 1)
	file="../complete1_$bench.cfg"
	cp config1.cfg $file
	sed -i 's/<pintool>/count_endbr64/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<out_dir>@$OUTPUT_DIR@g" $file
	sed -i "s@<log_file>@$COMPLETE_LOG@g" $file

	# Pruned pintool command (Grouping 1)
	file="../pruned1_$bench.cfg"
	cp config1.cfg $file
	sed -i 's/<pintool>/count_endbr64_pruned/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<out_dir>@$OUTPUT_DIR@g" $file
	sed -i "s@<log_file>@$PRUNED_LOG@g" $file

	# Complete pintool command (Grouping 2)
	file="../complete2_$bench.cfg"
	cp config2.cfg $file
	sed -i 's/<pintool>/count_endbr64/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<out_dir>@$OUTPUT_DIR@g" $file
	sed -i "s@<log_file>@$COMPLETE_LOG@g" $file

	# Pruned pintool command (Grouping 2)
	file="../pruned2_$bench.cfg"
	cp config2.cfg $file
	sed -i 's/<pintool>/count_endbr64_pruned/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<out_dir>@$OUTPUT_DIR@g" $file
	sed -i "s@<log_file>@$PRUNED_LOG@g" $file
done
