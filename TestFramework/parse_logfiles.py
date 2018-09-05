#!/usr/bin/env python3

bench_events = {}
complete_events = {}
pruned_events = {}
pintool_outputs = {}

dicts = {'benchs': bench_events,
         'complete': complete_events,
         'pruned': pruned_events}

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
                    if bench not in dicts[pintool]:
                        outputs[pintool][bench] = ('', '')
                    else:
                        logfile = open('overhead_outputs/' + pintool + '/' + bench + \
                                '.log', 'r')
                        
                        line = logfile.readline()
                        insts = line.strip().split(':')[1]
                        line = logfile.readline()
                        rets = line.strip().split(':')[1]
                        
                        if insts.isalnum() and rets.isalnum():                        
                            outputs[pintool][bench] = (insts, rets)
                        else:
                            outputs[pintool][bench] = ('', '')
                        
                        logfile.close()

        return outputs


def parse_logs():
        '''
                Parse the logfiles, retrieving event counting for the complete and
                pruned Pintools and for the benchmarks without Pin.

                This function changes the global dictionaries:
                    complete_events
                    pruned_events
                    bench_events
        '''
        
        global dicts

        files = ['complete', 'pruned', 'benchs']

        for f in files:
            logfile = open(f + '.log', 'r')
            cur_dict = dicts[f]

            line = logfile.readline()
            while line:
                if 'Performance' in line:
                    args = line.split()
                    current_bench = list(filter(lambda x: 'exe' in x, args))[0]
                    current_bench = current_bench.split('.')[1].strip('/')

                    if current_bench not in cur_dict:
                        cur_dict[current_bench] = {}
                else:
                    args = line.split()
                    if len(args) == 6 or len(args) == 7 or len(args) == 2 or len(args) == 3: # event report
                        value = args[0].replace(',', '')
                        event = args[1]

                        if len(args) == 6:
                            stdev = args[4]
                        elif len(args) == 7:
                            stdev = args[5]
                        else:
                            stdev = '0.00%'

                        if event == 'instructions':
                            current_weight = value
                        else:
                            cur_dict[current_bench][event] = (current_weight, \
                                    value)
                            #        value + '(+-' + stdev + ')')

                line = logfile.readline()

            logfile.close()


def get_value(d, bench, event, i=None):
    if bench in d:
        if event in d[bench]:
            if i is not None:
                if d[bench][event][i] is None:
                    return ''
                else:
                    return d[bench][event][i]
            else:
                if d[bench][event] is None:
                    return ''
                else:
                    return d[bench][event]
        else:
            return ''
    else:
        return ''

def print_results(benchs):
        '''
                Dump runtime results to file.

                @benchs: (string list) List with the benchmark names.
        '''

        output_file = open('output.csv', 'w')

        output_file.write('Bench,INSTs (Pin),RETs (Pin),Instructions (perf)')
        output_file.write(',cpu-clock,r8888,INSTs (Pin),RETs (Pin)')
        output_file.write(',Instructions (perf),cpu-clock,r8888,INSTs')
        output_file.write(',cpu-clock,r8888,r8889,INSTs')
        output_file.write(',iTLB-load-misses,r8488,r8489,INSTs,r8888,r8889')
        output_file.write(',ra088,INSTs,ra089\n')

        group1 = ['iTLB-load-misses', 'r8488', 'r8489']
        group2 = ['r8888', 'r8889', 'ra088']
        group3 = ['ra089']
        group4 = ['cpu-clock', 'r8888', 'r8889']

        groups = [group4, group1, group2, group3]

        for bench in sorted(benchs):
            output_file.write(bench)

            # Pruned pintool
            values = get_value(pintool_outputs, 'pruned', bench)
            output_file.write(',' + values[0] + ',' + values[1])
            inst = get_value(pruned_events, bench, 'cpu-clock', 0)
            cpu_clock = get_value(pruned_events, bench, 'cpu-clock', 1)
            r8888 = get_value(pruned_events, bench, 'r8888', 1)
            output_file.write(',' + inst + ',' + cpu_clock + ',' + r8888) 

            # Complete pintool
            values = get_value(pintool_outputs, 'complete', bench)
            output_file.write(',' + values[0] + ',' + values[1])
            inst = get_value(complete_events, bench, 'cpu-clock', 0)
            cpu_clock = get_value(complete_events, bench, 'cpu-clock', 1)
            r8888 = get_value(complete_events, bench, 'r8888', 1)
            output_file.write(',' + inst + ',' + cpu_clock + ',' + r8888)

            # No instrumentation
            for group in groups:
                first_event = group[0]
                inst = get_value(bench_events, bench, first_event, 0)
                output_file.write(',' + inst)

                for event in group:
                    value = get_value(bench_events, bench, event, 1)
                    output_file.write(',' + value)

            output_file.write('\n')

        output_file.close()
               

def main():
        global pintool_outputs
        parse_logs()
        benchs = [x for x in bench_events]
        pintool_outputs = parse_pintools_outputs(benchs)
        print_results(benchs)


main()
