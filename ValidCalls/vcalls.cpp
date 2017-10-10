/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * vcalls.cpp: Pintool used to determine valid calls in an application's
 * execution.
 */

#include "pin.H"

#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <sstream>
#include <utility>

typedef pair<string, string> Call;

using namespace std;

/**
 * Global Variables.
 */
static ofstream outputFile;
set<Call> calls;

/**
 * Prints the correct usage of the pintool.
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
 * Initializes the necessary data for the Pintool.
 *
 * @callListFileName: Input file name.
 * @return: A set that contains the memory locations of the CALL instructions
 * in the input file and their hexadecimal dump.
 */
void readInputData(string callListFileName) {
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
		
		Call newCall(addr, dump);
		calls.insert(newCall);
	}
		
    callListFile.close();
	return;
}

/**
 * Analysis function for RET instructions.
 */
VOID doRet() { // TODO
    
}

/**
 * For each trace in the application's execution flow, looks for CALLs and RETs,
 * calling the proper analysis function for each.
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
 * Performs necessary operations when the instrumented application is about to
 * end execution.
 */
VOID Fini(INT32 code, VOID *v) { // TODO
	set<Call> :: iterator it;
	
	for (it = calls.begin(); it != calls.end(); it++) {
		Call c = *it;
		cerr << c.first << ": " << c.second << endl;
	}
	
    //outputFile.close();
}

int main(int argc, char *argv[])
{
    // Gets the input file name from the command line (-i flag).
    KNOB<string> inFileKnob(KNOB_MODE_WRITEONCE, "pintool", "i",
        "call.txt", "Input file name -- this file must contain the list of"
        "addresses in memory that contain CALL instructions");

    // Gets the output file name from the command line (-o flag).
    //KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
    //    "pintool.out", "Output file name");
	
    // Starts Pin and checks parameters.
    if (PIN_Init(argc, argv)) {
        return PrintUsage();
    }

    // Gets CALL addresses.
    readInputData(inFileKnob.Value().c_str());
	if (calls.empty()) return -1;

    // Opens the output file.
    //outputFile.open(outFileKnob.Value().c_str(), \
    //    std::ofstream::out | std::ofstream::app);

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}