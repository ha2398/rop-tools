/**
 * @author: Hugo Sousa (hsousa@gmx.com)
 *
 * count_endbr64.cpp: Pruned version of the pintool used to count the indirect
 * branch targets that are the endbr64 (0xF30F1EFA) instruction.
 */

#include "pin.H"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;

// Get the output file name from the command line.
KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
	"count_endbr64_pruned.out", "Output file name.");

/**
 * Global variables.
 */

ofstream outputFile;
UINT32 ENDBR64 = 0xFA1E0FF3;

unsigned long indirectBrachesOrCalls = 0;
unsigned long indirectBrachesOrCallsFollowedByEndbr64 = 0;

/**
 * Analysis function for indirect JMPs and CALLs. Check if the branch (or
 * call) target address is an endbr64 instruction.
 */
VOID doIndirectBranchOrCall(ADDRINT target) {
	UINT32 targetBytes = 0;
	indirectBrachesOrCalls++;
}

/**
 * Instrument the code, adding calls to doIndirectBranchOrCall for every
 * indirect JMP or CALL.
 */
VOID InstrumentCode(TRACE trace, VOID *v) {
	// Loop through all BBLs.
	for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
		INS tail = BBL_InsTail(bbl);

		// Look for indirect .
		if (INS_IsIndirectBranchOrCall(tail) && !INS_IsRet(tail)) {
			INS_InsertCall(tail, IPOINT_BEFORE,
				(AFUNPTR) doIndirectBranchOrCall, IARG_BRANCH_TARGET_ADDR,
				IARG_END);
		}
	}
}

/**
 * Perform necessary operations when the instrumented application is about to
 * end execution.
 */
VOID Fini(INT32 code, VOID *v) {
	double ratio = (double(indirectBrachesOrCallsFollowedByEndbr64) / double(indirectBrachesOrCalls)) * 100;
	outputFile << "Indirect branches or calls,Followed by ENDBR64" << endl;
	outputFile << indirectBrachesOrCalls << ",";
	outputFile << indirectBrachesOrCallsFollowedByEndbr64;
	outputFile << " (" << setprecision(6) << ratio << "%)" << endl;
	outputFile.close();
}

int main (int argc, char *argv[]) {
	// Start Pin and check parameters.
	if (PIN_Init(argc, argv)) {
		cerr << "Failed to start Pin." << endl;
		return 1;
	}

	// Open output file.
	outputFile.open(outFileKnob.Value().c_str(), ios_base::app);

	TRACE_AddInstrumentFunction(InstrumentCode, NULL);
	PIN_AddFiniFunction(Fini, 0);
	PIN_StartProgram();

	return 0;
}