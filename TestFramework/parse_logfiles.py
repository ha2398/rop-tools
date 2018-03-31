#!/usr/bin/env python3


def parse_pintools_outputs(benchs):
	'''
		Read the outputs produced by the pintools.

		@benchs: List of benchmarks.

		@return: One dictionary with format
			(pintool -> (benchmark -> (instructions, RET instructions))).
	'''

	pintools = ['complete', 'pruned']
	outputs = {'complete': {}, 'pruned': {}}

	for pintool in pintools:
		for bench in benchs:
			logfile = open('overhead_outputs/' + pintool + '/' + bench + \
				'.exe.log')

			insts = int(readline.split()[1])
			rets = int(readline.split()[1])

			outputs[pintool][bench] = (insts, rets)
			logfile.close()

	return outputs


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


def print_results(file, complete_runtimes, pruned_runtimes, pintools_outputs):
	'''
		Dump runtime results to file.

		@file: File to dump results to.
		@complete_runtimes: Dictionary with runtimes for Complete pintool.
		@pruned_runtimes: Dictionary with runtimes for Pruned pintool.
		@pintools_outputs: Dictionary with format
			(pintool -> (benchmark -> (instructions, RET instructions))).
	'''

	file.write('Benchmark\tName\tComplete Runtime (s)\t' + \
		'Instructions (Complete)\tRETs (Complete)\t' + \
		'Pruned Runtime(s)\tInstructions (Pruned)\t' + \
		'RETs (Pruned)\n')

	counter = 1
	for bench, runtime in sorted(complete_runtimes.items()):
		runtime_complete = runtime
		runtime_pruned = pruned_runtimes[bench]

		complete_output = pintools_outputs['complete'][bench]
		complete_insts = complete_output[0]
		complete_rets = complete_output[1]

		pruned_output = pintools_outputs['pruned'][bench]
		pruned_insts = pruned_output[0]
		pruned_rets = pruned_output[1]

		file.write('{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\n'.format(counter, bench, \
			runtime_complete, complete_insts, complete_rets, runtime_pruned, \
			pruned_insts, pruned_rets))

		counter += 1


def main():
	complete = open('complete.log', 'r')
	pruned = open('pruned.log', 'r')
	output = open('overhead.log', 'w')

	complete_runtimes = parse_file(complete)
	pruned_runtimes = parse_file(pruned)

	benchs = [x for x in complete_runtimes]
	outputs = parse_pintools_outputs(benchs)

	print_results(output, complete_runtimes, pruned_runtimes)

	complete.close()
	pruned.close()
	output.close()

main()