#!/usr/bin/python2

# Imports

from __future__ import print_function
import sys

'''
@author: Hugo Sousa (hugosousa@dcc.ufmg.br)

valid_gadgets.py: This script gets 3 file as inputs:
	- rop: Mona generated text file with a list of interesting gadgets for an
	exploit based on a specific application.
	- calls: Mona generated text file with the list of all CALL instructions
	in a particular application.
	- valid calls: Text file with the list of all valid CALL instructions
	in a particular application execution.

And prints the number of gadgets on the rop list, the number of CALL
istructions on the call list, the number of gadgets that are preceded by
CALL instructions and the number of gadgets that are preceded by valid CALL
instructions.
'''

# Constants
NUM_ARGS = 4
USAGE_STR = 'Usage: valid_gadgets.py <rop> <calls> <valid_calls>'

# Indeces for the script's arguments.
ROP_ARG = 1
CALL_ARG = 2
VCALL_ARG = 3

# Size, in bytes, of each call instruction, by opcode.
CALL_SIZES = {'E8': [3, 5], '9A': [5, 7], 'FF': [2, 3, 4, 6, 7]}
MIN_CALL_SIZE = 2
MAX_CALL_SIZE = 7

addr_BASE = 16

# Functions


def print_usage():
	''' Prints the program's correct usage. '''
	print(USAGE_STR, file=sys.stderr)


def check_args():
	''' Checks the number of arguments passed to the program. '''
	if len(sys.argv) != NUM_ARGS:
		print('[Error] Invalid number of arguments.', file=sys.stderr)
		print_usage()
		exit()


def open_files(rop_filename, calls_filename, vcalls_filename):
	''' Tries to open the input files.
		@rop_filename: gadgets list file name.
		@calls_filename: call list file name.
		@vcalls_filename: valid call list file name.
		@return: rop file, call file, valid calls file.'''
	try:
		gadgets_file = open(rop_filename, 'r')
		calls_file = open(calls_filename, 'r')
		vcalls_file = open(vcalls_filename, 'r')
	except:
		print('[Error] Couldn\'t open input files.', file=sys.stderr)
		exit()
	else:
		return gadgets_file, calls_file, vcalls_file


def trim_rop_list(gadgets_file):
	''' Places the file pointer at the beginning of the actual gadget list
		on the file.
		@gadgets_file: File with gadgets list. '''
	next_line = gadgets_file.readline()
	while ('gadgets' not in next_line):
		next_line = gadgets_file.readline()

	next_line = gadgets_file.readline()


def get_gadgets(gadgets_file):
	''' Processes gadgets in the gadgets list file. 
		@gadgets_file: file with the gadgets list.
		@return: The list containing the gadgets' addresses. '''
	trim_rop_list(gadgets_file)
	
	gadgets_addr = []

	next_gadget = gadgets_file.readline()
	while (next_gadget != ''):
		if ' : ' in next_gadget:
			gadgets_addr.append(next_gadget.split()[0])
		
		next_gadget = gadgets_file.readline()

	return gadgets_addr


def trim_call_list(calls_file, vcalls_file):
	''' Place the file pointer at the beginning of the actual call list on the
		file.
		@calls_file: File with call list.
		@vcalls_file" File with valid calls list. '''

	# CALL list
	DELIM_SIZE = 137
	delim = '-' * DELIM_SIZE
	delim_counter = 0
	DELIM_NUM = 4

	next_line = calls_file.readline()
	while delim_counter < DELIM_NUM:
		if delim in next_line:
			delim_counter += 1
		if delim_counter != DELIM_NUM:
			next_line = calls_file.readline()

	# Valid CALL list.
	for i in range(10):
		vcalls_file.readline()


def get_calls(calls_file, vcalls_file):
	''' Processes call instructions in the gadgets call file.
		@calls_file: file with the call list.
		@vcalls_file: file with the valid call list.
		@return: Call instruction dictionary. Keys: addresses, values: dump.
				 Valid call instruction dictionary. Same structure. '''

	trim_call_list(calls_file, vcalls_file)

	calls = {}
	valid_calls = {}

	# CALLs
	next_call = calls_file.readline()
	while (next_call != ''):
		if ' : ' in next_call:
			strings = next_call.split()
			address = strings[0]

			dump = strings[2] if strings[2] != ':' else strings[3]

			calls[address] = dump
		
		next_call = calls_file.readline()

	# Valid CALLs
	next_call = vcalls_file.readline()
	while (next_call != ''):
		strings = next_call.split(':')
		address = '0x' + strings[0]
		dump = strings[1].strip().upper()
		valid_calls[address] = dump
		next_call = vcalls_file.readline()

	return calls, valid_calls


def is_preceded_by_call(address, calls):
	''' Checks if a given gadget is preceded by a call instruction.
		@address: Hexademical address of the gadget.
		@calls: Collection with all call instructions addresses.
		@return: True if the gadget is preceded by call and False otherwise.
		'''
	''' Hence, for each gadget, we need to check 6 addresses, the ones
		obtained by	subtracting x bytes from the gadget address, where x is an 
		integer in from 2 to 7, inclusive. '''
	for offset in range(MIN_CALL_SIZE, MIN_CALL_SIZE+1):
		candidate = hex(int(address, addr_BASE) - offset)
		if candidate in calls:
			call_opcode = calls[candidate][:2]
			if offset in CALL_SIZES[call_opcode]:
				return True

	return False

def is_preceded_by_valid_call(address, valid_calls):
	''' Checks if a given gadget is preceded by a call instruction.
		@address: Hexademical address of the gadget.
		@valid_calls: Collection with valid call instructions addresses.
		@return: True if the gadget is preceded by valid call and False otherwise.
		'''
	''' Hence, for each gadget, we need to check 6 addresses, the ones
		obtained by	subtracting x bytes from the gadget address, where x is an 
		integer in from 2 to 7, inclusive. '''
	for offset in range(MIN_CALL_SIZE, MIN_CALL_SIZE+1):
		candidate = hex(int(address, addr_BASE) - offset)
		if candidate in valid_calls:
			call_opcode = valid_calls[candidate][:2]
			if offset in CALL_SIZES[call_opcode]:
				return True

	return False


def filter_gadgets(gadgets_addr, calls, valid_calls):
	''' Filters gadgets preceded by call instructions. 
		@gadgets_addr: Collection with gadgets' addresses.
		@calls: Collection with call instructions' addresses.
		@valid_calls: Collection with valid call instructions' addresses.
		@return: A list of gadgets preceded by call instructions. '''
	call_gadgets = filter(lambda x: is_preceded_by_call(x, calls), \
		gadgets_addr)

	valid_call_gadgets = filter(lambda x: is_preceded_by_valid_call(x, \
		valid_calls), gadgets_addr)

	return call_gadgets, valid_call_gadgets


def main():
	''' Main script. '''
	check_args()
	gadgets_file, calls_file, vcalls_file = \
		open_files(sys.argv[ROP_ARG], sys.argv[CALL_ARG], sys.argv[VCALL_ARG])

	gadgets_addr = get_gadgets(gadgets_file)

	calls, valid_calls = get_calls(calls_file, vcalls_file)

	call_gadgets, valid_call_gadgets = \
		filter_gadgets(gadgets_addr, calls, valid_calls)

	print('Number of gadgets: ' + str(len(gadgets_addr)))
	print('Number of calls: ' + str(len(calls)))
	print('Number of gadgets preceded by calls: ' + str(len(call_gadgets)))
	print('Number of gadgets preceded by valid calls: ' + \
		str(len(valid_call_gadgets)))

	gadgets_file.close()
	calls_file.close()
	vcalls_file.close()


# Main script


main()