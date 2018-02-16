/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * lbrmatch.cpp: Pintool used to simulate the LBR hardware and check for CALL
 * matches using it.
 */

#include "pin.H"

#include <iostream>
#include <fstream>

using namespace std;

// Get the output file name from the command line.
KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
	"lbr_out.log", "Output file name");

// Get the number of entries on each LBR.
KNOB<unsigned int> lbrSizeKnob(KNOB_MODE_WRITEONCE, "pintool", "s",
	"32", "Number of entries on each LBR");

/**
 * LBR (Last Branch Record) data structure.
 */
class LBR {
private:
	ADDRINT *buffer;
	unsigned int head, tail, size;
public:
	LBR(unsigned int size) {
		this->size = size;
		head = tail = 0;
		buffer = (ADDRINT*) malloc(sizeof(ADDRINT) * (size + 1));
	}
	
	bool empty() {
		return (head == tail);
	}
	
	void put(ADDRINT item) {
		buffer[head] = item;
		head = (unsigned int) (head + 1) % size;
		
		if (head == tail)
			tail = (unsigned int) (tail + 1) % size;
	}
	
	void pop() {
		if (empty())
			return; 
		
		head = (unsigned int) (head - 1) % size;
	}
	
	ADDRINT getLastCall() {
		if (empty())
			return 0;
		
		unsigned int index = (unsigned int) (head - 1) % size;
		
		return buffer[index];
	}
};

/**
 * Global Variables.
 */

const string done("\t- Done.");
static ofstream outputFile; // Output file

LBR callLBR(lbrSizeKnob.Value()); // CALL LBR
int callLBRMatches = 0;

LBR indirectCallLBR(lbrSizeKnob.Value()); // Indirect CALLs LBR
int indirectCallLBRMatches = 0;

unsigned long retCount = 0; // Number of RETs found
unsigned long directCallCount = 0; // Number of direct CALLs found
unsigned long indirectCallCount = 0; // Number of indirect CALLs found

VOID doRET(ADDRINT returnAddr) {
	/**
	 * Pintool analysis function for return instructions.
	 *
	 * @ctxt: Pointer to Pin's current CONTEXT object
	 * @returnAddr: Return address.
	 */
	 
	ADDRINT lastCall;
	retCount++;
	
	/**
	 * LBR Matches Experiment.
	 *
	 * Candidate CALL can be from 2 to 7 bytes before the return address.
	 */
	
	lastCall = callLBR.getLastCall();
	for (int i = 2; i <= 7; i++) {
		ADDRINT candidate = returnAddr - i;
		
		if (candidate == lastCall) {
			callLBRMatches++;
			break;
		}
	}
	
	lastCall = indirectCallLBR.getLastCall();
	for (int i = 2; i <= 7; i++) {
		ADDRINT candidate = returnAddr - i;
		
		if (candidate == lastCall) {
			indirectCallLBRMatches++;
			break;
		}
	}
	
	callLBR.pop();
}

VOID doDirectCALL(ADDRINT addr) {
	/**
	 * Pintool analysis fuction for direct call instructions.
	 *
	 * @addr: The instruction's address.
	 */
	
	directCallCount++;
	callLBR.put(addr);
}

VOID doIndirectCALL(ADDRINT addr) {
	/**
	 * Pintool analysis fuction for indirect call instructions.
	 *
	 * @addr: The instruction's address.
	 */
	
	indirectCallCount++;
	callLBR.put(addr);
	indirectCallLBR.put(addr);
}

VOID InstrumentCode(TRACE trace, VOID *v) {
    /**
	 * Pintool instrumentation function.
	 * 
     * Each Basic Block (BBL) has a single entrace point and a single exit one
     * as well. Hence, CALL and RET instructions will only be found at the end
     * of these BBLs.
     */
	 
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        INS tail = BBL_InsTail(bbl);
		
		if (INS_IsRet(tail)) {
			INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR) doRET, \
				IARG_BRANCH_TARGET_ADDR, IARG_END);
		} else if (INS_IsCall(tail)) {
			if (INS_IsDirectCall(tail))
				INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR) doDirectCALL, \
					IARG_INST_PTR, IARG_END);
			else
				INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR) doIndirectCALL, \
					IARG_INST_PTR, IARG_END);
		}
			
    }
}

void printExperimentReport() {
	/**
	 * Print the report for the LBR experiment.
	 */
	
	outputFile << "Reports for experiment \"LBR Match\" with " << \
		lbrSizeKnob.Value() << " entries" << endl;
	outputFile << "[+] Number of RET instructions:" << endl << \
		"\t" << retCount << endl;
	outputFile << "[+] Number of Direct CALL instructions:" << endl << \
		"\t" << directCallCount << endl;
	outputFile << "[+] Number of Indirect CALL instructions:" << endl << \
		"\t" << indirectCallCount << endl;
	outputFile << "[+] CALL LBR Matches:" << endl << \
		"\t" << callLBRMatches << endl;
	outputFile << "[+] Indirect CALL LBR Matches:" << endl << \
		"\t" << indirectCallLBRMatches << endl;
}

VOID Fini(INT32 code, VOID *v) {
	/**
	 * Perform necessary operations when the instrumented application is about
	 * to end execution.
	 */
	
	cerr << done << endl;
	printExperimentReport();
    outputFile.close();
}

int main(int argc, char *argv[])
{
    // Start Pin and checks parameters.
    if (PIN_Init(argc, argv)) {
        cerr << "[Error] Could not start Pin." << endl;
		return -1;
    }
	
	// Open the output file.
    outputFile.open(outFileKnob.Value().c_str());

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
	cerr << "[+] Running application." << endl;
    PIN_StartProgram();

    return 0;
}