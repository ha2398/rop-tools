/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * vcalls.cpp: Pintool used to determine valid calls in an application's
 * execution.
 */

#include "pin.H"

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
 * Return the target address of a CALL instruction represented by string
 * @dump, the instruction`s hexadecimal dump.
 */
UINT64 getCallTarget(string dump) { // TODO
	string opcode = dump.substr(0, 2);
	
	switch(getOpcodeCode(opcode)) {
	case opE8: // Call near, relative
		
		break;
		
	case op9A: // Call far, absolute
	
		break;
		
	case opFF: // Call near, absolute indirect OR Call far, absolute indirect.
		
		break;
	}
	
	return 0;
}

/**
 * Analysis function for RET instructions.
 */
VOID doRet() { // TODO
	
}

/**
 * For each trace in the application's execution flow, look for RETs.
 */
VOID InstrumentCode(TRACE trace, VOID *v) { // TODO
    /**
     * Each Basic Block (BBL) has a single entrace point and a single exit one
     * as well. Hence, CALL and RET instruction will only be found at the end
     * of these BBLs.
     */
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        INS tail = BBL_InsTail(bbl);

        if (INS_IsRet(tail)) { // Instruments RETs
            INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)doRet, IARG_END);
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