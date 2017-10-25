/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * vcalls.cpp: Pintool used to determine valid calls in an application's
 * execution.
 */

#include "pin.H"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

using namespace std;

enum callOpcode {
	opE8, op9A, opFF
};

/**
 * Global Variables.
 */
static ofstream outputFile;
map<string, string> calls;

/**
 * Print the correct usage of the pintool.
 */
INT32 PrintUsage() {
    cerr << "\nUsage: pin -t <Pintool> [-o <OutputFileName> [-i"
        " <InputFileName>] -- <Application>\n\n"
        "Options:\n"
        "\t-o\t<OutputFileName>\t"
        "Indicates the name of the output file (default: pintool.out)\n"
        "\t-i\t<InputFileName>\t"
        "Indicates the name of the input file name. This file must contain the"
        " memory locations of the CALL instructions in the application"
        " (default: call.txt)\n\n";

    return -1;
}

/**
 * Initialize the necessary data for the Pintool.
 *
 * @callListFileName: Input file name.
 * @return: A set that contains the memory locations of the CALL instructions
 * in the input file and their hexadecimal dump.
 */
VOID readInputData(string callListFileName) {
    ifstream callListFile;
	callListFile.open(callListFileName.c_str());
	
	if (!callListFile) {
		cerr << "[Error] Couldn't open call list file." << endl;
		return;
	}

    string line;
    while (getline(callListFile, line)) {
		istringstream iss(line);
		string addr, dump;
		
		getline(iss, addr, ' ');
		getline(iss, dump, ' ');
		
		calls.insert( pair<string, string>(addr, dump) );
	}
		
    callListFile.close();
	return;
}

/**
 * Reverse the byte order of a string that represents an hexadecimal byte flow.
 */
string reverseByteOrder(string const& bytes) {
	string result;
	result.reserve(bytes.size());
	
	for (size_t i = bytes.size(); i != 0; i -= 2)
		result.append(bytes, i-2, 2);
	
	// Remove zeros from beginning.
	int index = 0;
	while (result.at(index) == '0')
		index++;

	result = result.substr(index);
	
	return result;
}

/**
 * Get the CALL instruction opcode code.
 */
callOpcode getOpcodeCode(string const& opcode) {
	if (opcode == "E8")
		return opE8;
	if (opcode == "9A")
		return op9A;
	else
		return opFF;
}

/**
 * Return the target address of a CALL instruction.
 * @ctxt: Pointer to CPU context Pin object.
 * @addr: the instruction's address.
 * @dump: the instruction's hexadecimal dump.
 */
string getCallTarget(const CONTEXT *ctxt, string addr, string dump) { // TODO
	string target; // CALL target
	string opcode = dump.substr(0, 2); // CALL opcode
	string operand; // Operand hex string for some CALL types
	
	int segment; // Segment for ptrX:Y
	int offset; // Offset for ptrX:Y
	
	stringstream ss; // Used for hex <-> decimal conversions
	
	int dest; // CALL operand
	int operandSize; // Operand size for some CALL types
	int EIP; // Instruction pointer value
	
	operand = reverseByteOrder(dump.substr(2));
	
	switch (getOpcodeCode(opcode)) {
	case opE8: // Call near, relative
		operandSize = (operand.length()/2) * 8; // Operand size in bits
		
		EIP = strtoul(addr.c_str(), NULL, 16);
		EIP += (dump.length()/2);
		
		dest = strtoul(operand.c_str(), NULL, 16);
		
		switch (operandSize) {
		case 64:
		case 32:
			ss << hex << EIP + dest;	
			break;
		case 16:
			ss << hex << ((EIP + dest) & 0x0000FFFF);
			break;
		}
		
		break;
		
	case op9A: // Call far, absolute
		segment = strtoul(operand.substr(0, 4).c_str(), NULL, 16);
		offset = strtoul(operand.substr(4).c_str(), NULL, 16);
		
		ss << hex << (segment * 0x10) + offset;
		
		break;
		
	case opFF: // Call near, absolute indirect OR Call far, absolute indirect.
		// TODO
		target = "0";
		break;
	}
	
	target = "0x" + ss.str();
	cerr << target << endl;
	return target;
}

/**
 * Analysis function for RET instructions.
 */
VOID doRet(VOID *ip, const CONTEXT *ctxt) { // TODO
	
}

/**
 * For each trace in the application's execution flow, look for RETs.
 */
VOID InstrumentCode(TRACE trace, VOID *v) {
    /**
     * Each Basic Block (BBL) has a single entrace point and a single exit one
     * as well. Hence, CALL and RET instructions will only be found at the end
     * of these BBLs.
     */
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        INS tail = BBL_InsTail(bbl);

        if (INS_IsRet(tail)) { // Instruments RETs
            INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)doRet, IARG_INST_PTR, IARG_CONTEXT, IARG_END);
        }	
    }
}

/**
 * Perform necessary operations when the instrumented application is about to
 * end execution.
 */
VOID Fini(INT32 code, VOID *v) { // TODO
    //outputFile.close();
}

int main(int argc, char *argv[])
{
    // Get the input file name from the command line (-i flag).
    KNOB<string> inFileKnob(KNOB_MODE_WRITEONCE, "pintool", "i",
        "call.txt", "Input file name -- this file must contain the list of"
        "addresses in memory that contain CALL instructions");

    //Get the output file name from the command line (-o flag).
    //KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
    //    "pintool.out", "Output file name");
	
    // Start Pin and checks parameters.
    if (PIN_Init(argc, argv)) {
        return PrintUsage();
    }

    // Get CALL addresses.
    readInputData(inFileKnob.Value().c_str());
	if (calls.empty()) return -1;

    // Open the output file.
    //outputFile.open(outFileKnob.Value().c_str(), \
    //    std::ofstream::out | std::ofstream::app);

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}