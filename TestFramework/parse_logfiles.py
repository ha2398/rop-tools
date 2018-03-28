#!/usr/bin/env python3


def parse_file(file):
	'''
		Parse a logfile, retrieving runtimes for benchmarks.

		@file: File to parse.

		@return: A dictionary benchmark (string) -> runtime (float).
	'''

	runtimes = {}

	# Ignore headers
	file.readline()

	line = file.readline()
	# Parse log file.
	while line:
		line_args = line.split()
		runtime = float(line_args[3])
		benchmark = line_args[9].split('/')[-1]

		runtimes[benchmark] = runtime
		line = file.readline()

	return runtimes


def print_results(file, complete_runtimes, pruned_runtimes):
	'''
		Dump runtime results to file.

		@file: File to dump results to.
		@complete_runtimes: Dictionary with runtimes for Complete pintool.
		@pruned_runtimes: Dictionary with runtimes for Pruned pintool.
	'''

	file.write('Benchmark\tName\tComplete Runtime (s)\tPruned Runtime (s)\n')
	counter = 1
	for bench, runtime in sorted(complete_runtimes.items()):
		runtime_complete = runtime
		runtime_pruned = pruned_runtimes[bench]
		file.write('{}\t{}\t{}\t{}\n'.format(counter, bench, \
			runtime_complete, runtime_pruned))
		counter += 1


def main():
	complete = open('complete.log', 'r')
	pruned = open('pruned.log', 'r')
	output = open('overhead.log', 'w')

	complete_runtimes = parse_file(complete)
	pruned_runtimes = parse_file(pruned)

	print_results(output, complete_runtimes, pruned_runtimes)

	complete.close()
	pruned.close()
	output.close()

main()