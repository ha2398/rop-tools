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

/**
 * Global Variables.
 */
static std::ofstream outputFile;
UINT64 callCount = 0;

/**
 * Prints the correct usage of the pintool.
 */
INT32 PrintUsage() {
    cout << "\nUsage: pin -t <Pintool> [-o <OutputFileName> [-i"
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
 * in the input file.
 */
set<string> readInputData(string callListFileName) {
    ifstream callListFile(callListFileName.c_str());
    set<string> callAddr;

    string line;
    while (getline(callListFile, line)) {
        callAddr.insert(line);
    }

    callListFile.close();
    return callAddr;
}

/**
 * Analysis function for CALL instructions.
 */
VOID doCall(ADDRINT target) { // TODO
    callCount++;
    cout << "Target: " << target << endl;
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

        if (INS_IsCall(tail)) { // Instruments CALLs
            if (INS_IsDirectCall(tail)) { // Direct CALLs
                const ADDRINT target = INS_DirectBranchOrCallTargetAddress(tail);
                INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(doCall),
                    IARG_ADDRINT, target, IARG_END);
            } else { // Indirect CALLs
                INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(doCall),
                    IARG_BRANCH_TARGET_ADDR, IARG_END);
            }
        }	
    }
}

/**
 * Performs necessary operations when the instrumented application is about to
 * end execution.
 */
VOID Fini(INT32 code, VOID *v) { // TODO
    cout << "Call count: " << callCount << endl;
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
    //set<string> callAddr = readInputData(inFileKnob.Value().c_str());

    // Opens the output file.
    //outputFile.open(outFileKnob.Value().c_str(), \
    //    std::ofstream::out | std::ofstream::app);

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}