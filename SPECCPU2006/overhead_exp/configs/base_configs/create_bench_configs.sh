#!/bin/bash

declare -a benchs=("400.perlbench" "401.bzip2" "403.gcc" "429.mcf" "445.gobmk" "456.hmmer" "458.sjeng" "462.libquantum" "464.h264ref" "471.omnetpp" "473.astar" "483.xalancbmk" "999.specrand" "410.bwaves" "416.gamess" "433.milc" "434.zeusmp" "435.gromacs" "436.cactusADM" "437.leslie3d" "444.namd" "447.dealII" "450.soplex" "453.povray" "454.calculix" "459.GemsFDTD" "465.tonto" "470.lbm" "481.wrf" "482.sphinx3" "998.specrand")

ROOT_DIR="/home/hugo/SPEC_CPU2006v1.2/overhead_exp"
COMPLETE_LOG="$ROOT_DIR/complete.log"
PRUNED_LOG="$ROOT_DIR/pruned.log"
BENCH_LOG="$ROOT_DIR/benchs.log"

for bench in "${benchs[@]}"
do
	# Complete pintool command
	file="../complete_$bench.cfg"
	cp pin.cfg $file
	sed -i 's/<pintool>/complete/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<log_file>@$COMPLETE_LOG@g" $file

	# Pruned pintool command
	file="../pruned_$bench.cfg"
	cp pin.cfg $file
	sed -i 's/<pintool>/pruned/g' $file
	sed -i "s/<bench>/$bench/g" $file
	sed -i "s@<log_file>@$PRUNED_LOG@g" $file
done	

# Perf events commands
file="../perf_e1.cfg"
cp perf_e1.cfg $file
sed -i "s@<log_file>@$BENCH_LOG@g" $file

file="../perf_e2.cfg"
cp perf_e2.cfg $file
sed -i "s@<log_file>@$BENCH_LOG@g" $file

file="../perf_e3.cfg"
cp perf_e3.cfg $file
sed -i "s@<log_file>@$BENCH_LOG@g" $file
