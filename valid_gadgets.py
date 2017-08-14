#!/usr/bin/python2

# Imports

from __future__ import print_function
import sys

'''
@author: Hugo Sousa (hugosousa@dcc.ufmg.br)
valid_gadgets.py: This script gets one file as input:
	- rop: Mona generated text file with a list of interesting gadgets for an
	exploit.

And returns the number of gadgets on the rop list.
'''

# Constants
NUM_ARGS = 2
USAGE_STR = 'Usage: valid_gadgets.py <rop>'
ROP_ARG = 1

# Functions

def print_usage():
	''' Prints the program's correct usage. '''
	print(USAGE_STR, file=sys.stderr)

def check_args():
	''' Checks the n\umber of arguments passed to the program. '''
	if len(sys.argv) != NUM_ARGS:
		print('[Error] Invalid number of arguments.', file=sys.stderr)
		print_usage()
		exit()

def open_files(rop_filename):
	''' Tries to open the input files.
		@rop_filename: gadgets list file name.
		@return: rop file.'''
	try:
		gadgets_file = open(rop_filename, 'r')
	except:
		print('[Error] Couldn\'t open input files.', file=sys.stderr)
		exit()
	else:
		return gadgets_file

def trim_rop_list(gadgets_file):
	''' Places the file pointer at the beginning of the actual gadget list
		on the file.
		@gadgets_file: File with gadgets list. '''
	next_line = gadgets_file.readline()
	while ("gadgets" not in next_line):
		next_line = gadgets_file.readline()

	next_line = gadgets_file.readline()

def count_gadgets(gadgets_file):
	''' Counts the number of gadgets in the gadgets list file. 
		@gadgets_file: file with the gadgets list.
		@return: The number of gadgets found. '''
	gadgets = 0

	next_gadget = gadgets_file.readline()
	while (next_gadget != ''):
		if ':' in next_gadget:
			gadgets += 1
		next_gadget = gadgets_file.readline()

	return gadgets

def main():
	''' Main script. '''
	check_args()
	gadgets_file = open_files(sys.argv[ROP_ARG])
	trim_rop_list(gadgets_file)

	print('Number of Gadgets: ' + str(count_gadgets(gadgets_file)))

	gadgets_file.close()

# Main script

main()