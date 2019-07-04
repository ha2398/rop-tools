#!/usr/bin/env python3

complete_events = {}
pruned_events = {}
pintool_outputs = {}

dicts = {'complete': complete_events,
         'pruned': pruned_events}

separator = '\t'

def parse_pintools_outputs(benchs):
    '''
        Read the outputs produced by the pintools.

        @benchs: List of benchmarks.
        @return: One dictionary with format
                (pintool -> (benchmark -> (Indirect branches or calls,Followed by ENDBR64))).
    '''

    pintools = ['complete', 'pruned']
    pintools_names = {'complete': 'count_endbr64', 'pruned': 'count_endbr64_pruned'}
    outputs = {p: {} for p in pintools}

    for pintool in pintools:
            for bench in benchs:
                if bench not in dicts[pintool]:
                    outputs[pintool][bench] = ('', '')
                else:
                    with open('endbr64_outputs/{}/{}.log'.format(pintools_names[pintool], bench), 'r') as logfile:
                        while logfile.readline():
                            line = logfile.readline()
                            results = line.split(' ')[0].split(',')
                            branches = int(results[0])
                            endbr64_branches = int(results[1])

                            if bench in outputs[pintool]:
                                cur_values = outputs[pintool][bench]
                                outputs[pintool][bench] = (branches + cur_values[0], endbr64_branches + cur_values[1])
                            else:
                                outputs[pintool][bench] = (branches, endbr64_branches)

    return outputs


def is_event_report(args):
    '''
        Checks whether a line in a perf output file is an event report.

        @args: the split line to be checked.
        @return: true iff the line is an event report.
    '''

    acceptedNumArgs = [2, 5, 6, 7, 9, 11]
    return len(args) in acceptedNumArgs


def parse_logs():
    '''
        Parse the logfiles, retrieving event counting for the complete and
        pruned Pintools.
    '''
    
    global dicts
    files = ['complete', 'pruned']

    for f in files:
        logfile = open(f + '.log', 'r')
        cur_dict = dicts[f]

        line = logfile.readline()
        current_insts = 0
        current_cpu = 0
        while line:
            if 'Performance' in line:
                args = line.split()
                current_bench = list(filter(lambda x: '.log' in x, args))[0]
                current_bench = '.'.join(current_bench.split('/')[-1].split('.')[:2])

                if current_bench not in cur_dict:
                    cur_dict[current_bench] = {}
            else:
                args = line.split()
                length = len(args)

                if is_event_report(args): # event report
                    value = float(args[0])
                    event = args[1]

                    # if '(' in line:
                    #     stdev = args[length - 2]
                    # else:
                    #     stdev = '0.00%'

                    if 'instructions' in event:
                        current_insts = value
                    elif 'cpu-clock' in event:
                        current_cpu = value
                    else:
                        if event in cur_dict[current_bench]:
                            cur_values = cur_dict[current_bench][event]
                            cur_dict[current_bench][event] = (current_insts + cur_values[0], current_cpu + cur_values[1], value + cur_values[2])
                        else:
                            cur_dict[current_bench][event] = (current_insts, current_cpu, value)

            line = logfile.readline()

        logfile.close()


def print_results(benchs):
    '''
        Dump runtime results to file.

        @benchs: (string list) List with the benchmark names.
    '''

    output_file = open('output.csv', 'w')

    group1 = ['instructions:u','cpu-clock:u','r8488:u','r8489:u']
    group2 = ['instructions:u','cpu-clock:u','ra088:u','ra089:u']
    events = group1 + group2
    header = separator.join(events)

    output_file.write(separator.join(['Bench','Indirect branches or calls','Followed by ENDBR64']))
    # Events are printed in the following order: pruned, complete
    output_file.write('{}{}{}{}\n'.format(separator, header, separator, header))

    groupings = [group1[2:], group2[2:]]

    for bench in sorted(benchs):
        output_file.write(bench)

        # Pintool logs (only complete Pintool)
        values = pintool_outputs['complete'][bench]
        output_file.write('{}{}{}{}'.format(separator, values[0], separator, values[1]))

        # Pintool perf events
        for results in [pruned_events, complete_events]:
            for grouping in groupings:
                instructions = results[bench][grouping[0]][0]
                output_file.write('{}{}'.format(separator, instructions))
                cpu = results[bench][grouping[0]][1]
                output_file.write('{}{}'.format(separator, cpu))

                for event in grouping:
                    value = results[bench][event][2]
                    output_file.write('{}{}'.format(separator, value))

        output_file.write('\n')

    output_file.close()
               

def main():
        global pintool_outputs
        parse_logs()
        benchs = list(pruned_events.keys())
        pintool_outputs = parse_pintools_outputs(benchs)
        print_results(benchs)


main()
