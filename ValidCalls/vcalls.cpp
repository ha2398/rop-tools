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
unsigned long totalFound = 0;
unsigned long totalCorrect = 0;

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
 * Convert a hex string to a two's complement number (signed integer).
 * @hex: String that represents the number in two's complement.
 */
long hexToInt(string hex) {
	if (hex.empty())
		return 0;
	
	long number = strtoul(hex.c_str(), 0, 16);
	
	short index = 0;
	// Check if 0x prefix is present.
	if (hex[1] == 'x')
		index += 2;
	
	// Check if number is negative.
	if (hex[index] > '7')
		number = ~number + 1;
	
	return number;
}

/**
 * Return the target address of a CALL instruction.
 * @ctxt: Pointer to CPU context Pin object.
 * @addr: the instruction's address.
 * @dump: the instruction's hexadecimal dump.
 */
long getCallTarget(const CONTEXT *ctxt, ADDRINT addr, string dump) { // TODO
	string opcode = dump.substr(0, 2); // CALL opcode
	string operand; // Operand hex string for some CALL types
	
	long segment; // Segment for ptrX:Y
	long offset; // Offset for ptrX:Y
	
	long dest; // CALL operand
	unsigned short operandSize; // Operand size for some CALL types
	long IP; // Instruction pointer value
	
	unsigned short modRM; // ModR/M byte for FF CALLs.
	unsigned short mod; // Mod field from ModR/M byte.
	unsigned short reg; // Reg field from ModR/M byte.
	unsigned short RM; // R/M field from ModR/M byte.
	
	long disp; // Displacement
	ADDRINT regVal; // Holds temporary values obtained from registers.
	
	long target; // CALL target.
	unsigned long memOp; // Holds value read from memory.
	unsigned short memOpSize;//Holds size (in bytes) of value read from memory.
	
	operand = reverseByteOrder(dump.substr(2));
	
	switch (getOpcodeCode(opcode)) {
	case opE8: // Call near, relative
		operandSize = (operand.length()/2) * 8; // Operand size in bits
		
		IP = addr;
		IP += (dump.length()/2);
		dest = hexToInt(operand);
		
		switch (operandSize) {
		case 64:
		case 32:
			target = IP + dest;	
			break;
		case 16:
			target = ((IP + dest) & 0x0000FFFF);
			break;
		}
		
		break;
		
	case op9A: // Call far, absolute
		segment = hexToInt(operand.substr(0, 4));
		offset = hexToInt(operand.substr(4));
		
		target = (segment * 0x10) + offset;
		break;
		
	case opFF: // Call near, absolute indirect OR Call far, absolute indirect.
		// TODO
		
		modRM = hexToInt(operand.substr(0, 2));
		
		mod = (modRM & 0xC0) >> 6;
		reg = (modRM & 0x38) >> 3;
		RM = (modRM & 0x7);
		
		switch (RM) { // Select register to use
		case 0:
			regVal = PIN_GetContextReg(ctxt, REG_EAX);
			break;
		case 1:
			regVal = PIN_GetContextReg(ctxt, REG_ECX);
			break;
		case 2:
			regVal = PIN_GetContextReg(ctxt, REG_EDX);
			break;
		case 3:
			regVal = PIN_GetContextReg(ctxt, REG_EBX);
			break;
		case 4:
			if (mod < 3) { // Depends on following SIB byte.
			
			} else {
				regVal = PIN_GetContextReg(ctxt, REG_ESP);
			}
			break;
		case 5:
			if (mod > 0) {
				regVal = PIN_GetContextReg(ctxt, REG_EBP);
			} else { // Displacement
				
			}
		case 6:
			regVal = PIN_GetContextReg(ctxt, REG_ESI);
			break;
		case 7:
			regVal = PIN_GetContextReg(ctxt, REG_EDI);
			break;
		}
		
		if (mod < 3) { // Calculate displacement
			disp = hexToInt(operand.substr(2));
			memOpSize = reg * 2; // Empirical. DWORD, FWORD based on reg value.
			PIN_SafeCopy(&memOp, (ADDRINT *) regVal, memOpSize);
			target = memOp + disp;
		} else {
			
		}
		
		target = 0;
		break;
	default:
		target = 0;
	}
	
	return target;
}

/**
 * Analysis function for CALLs.
 */
VOID doCall(ADDRINT ip, ADDRINT target, const CONTEXT *ctxt) { // TODO
	totalFound++;

	stringstream ss;
	ss << hex << ip;
	string key("0x" + ss.str());
	
	auto search = calls.find(key);
	
	if (search == calls.end()) {
		outputFile << "Error!" << endl;
		return;
	}
	
	string dump = search->second;	
	ADDRINT calculatedTarget = getCallTarget(ctxt, ip, dump);

	if (calculatedTarget != target)
		outputFile << "Incorrect: " << dump.substr(0, 2) << endl;
	else
		totalCorrect++;
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

        if (INS_IsCall(tail)) { // Instrument CALLs
			if (INS_IsDirectBranchOrCall(tail)) { // Direct CALLs
				ADDRINT target = INS_DirectBranchOrCallTargetAddress(tail);
				
				INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)doCall, \
					IARG_INST_PTR, IARG_ADDRINT, target, IARG_CONTEXT, IARG_END);
			} else { // Indirect CALLs
				INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)doCall, \
					IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_CONTEXT, IARG_END);
			}
        }
    }
}

/**
 * Perform necessary operations when the instrumented application is about to
 * end execution.
 */
VOID Fini(INT32 code, VOID *v) {
	outputFile << "Precision rate: " << (totalCorrect/totalFound)*100 << "%" << endl;
    outputFile.close();
}

int main(int argc, char *argv[])
{
    // Get the input file name from the command line (-i flag).
    KNOB<string> inFileKnob(KNOB_MODE_WRITEONCE, "pintool", "i",
        "call.txt", "Input file name -- this file must contain the list of"
        "addresses in memory that contain CALL instructions");

    // Get the output file name from the command line (-o flag).
    KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
        "pintool.out", "Output file name");
	
    // Start Pin and checks parameters.
    if (PIN_Init(argc, argv)) {
        return PrintUsage();
    }

    // Get CALL addresses.
    readInputData(inFileKnob.Value().c_str());
	if (calls.empty()) {
		cerr << "[Error] Couldn't build call list." << endl;
		return -1;
	}

    // Open the output file.
    outputFile.open(outFileKnob.Value().c_str(), \
        std::ofstream::out | std::ofstream::app);

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}