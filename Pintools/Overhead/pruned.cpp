/**
 * @author: Hugo Sousa (hugosousa@dcc.ufmg.br)
 *
 * pruned.cpp: Pintool used to calculate the overhead of the funcionality
 * proposed as part of ROP detection.
 */

#include "pin.H"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

// Get the output file name from the command line.
KNOB<string> outFileKnob(KNOB_MODE_WRITEONCE, "pintool", "o", \
	"pruned_out.log", "Output file name");

// Get the number of entries on each LBR.
KNOB<unsigned int> lbrSizeKnob(KNOB_MODE_WRITEONCE, "pintool", "s",
	"32", "Number of entries on each LBR");

KNOB<double> TLBmissRateKnob(KNOB_MODE_WRITEONCE, "pintool", "m",
	"0.5", "TLB Miss Rate in percentage");

/**
 * LBR (Last Branch Record) data structure.
 */

/**
 * A LBR entry is composed by the address of the branch instruction and
 * a boolean that indicates whether this is a direct branch (true) or
 * indirect (false).
 */
typedef pair<ADDRINT, bool> LBREntry;

class LBR {
private:
	LBREntry *buffer;
	unsigned int head, tail, size;
public:
	LBR(unsigned int size) {
		this->size = size;
		head = tail = 0;
		buffer = (LBREntry*) malloc(sizeof(LBREntry) * (size + 1));
	}
	
	bool empty() {
		return (head == tail);
	}
	
	void put(LBREntry item) {
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
	
	LBREntry getLastEntry() {
		if (empty())
			return make_pair(0, false);
		
		unsigned int index = (unsigned int) (head - 1) % size;
		
		return buffer[index];
	}
};

/**
 * Global Variables.
 */
 
static ofstream outputFile; // Output file
LBR callLBR(lbrSizeKnob.Value()); // CALL LBR
int TLBmissInterval;
 
enum callOpcode {
	opE8 = 0, op9A, opFF
};

string callOpcodeStrings[3] = {
	string("e8"),
	string("9a"),
	string("ff")
};

// Rows -> Size in bytes.
// Columns -> 1 if correspondent opcode can lead to a CALL of that size.
// E8 | 9A | FF
int sizeToOpcodes[8][3] = {
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 1},
	{1, 0, 1},
	{0, 0, 1},
	{1, 1, 0},
	{0, 0, 1},
	{0, 1, 1},
};

unsigned long instCount = 0; // Total number of instructions
unsigned long retCount = 0; // Number of RETs found
unsigned long retsPrecededByValidICALL = 0;
unsigned long retsPrecededByValidDCALL = 0;
unsigned long retsPrecededByInvalidICALL = 0;
unsigned long retsPrecededByInvalidDCALL = 0;
unsigned long retsNotPrecededByCALLs = 0;

int TLBCounter = 0;

long int hexToInt(string in) {
	/**
	 * Convert a string that represents a sequence of bytes in hexadecimal
	 * notation to a two's complement number (signed integer) equivalent to the
	 * hex bytes.
	 * 
	 * @in: String that represents a sequence of bytes in hexadecimal notation.
	 * @return: Two's complement number (signed integer) equivalent to the
	 * input hex bytes.
	 */
	 
	int bits = in.length() * 4;
	char *endPtr;
	long long int result;

	result = strtoll(in.c_str(), &endPtr, 16);

	if (result >= (1LL << (bits - 1)))
		result -= (1LL << bits);

	return result;
}

bool isDirectCall(string dump) {
	/**
	 * Check if a given CALL instruction is a direct CALL.
	 *
	 * @dump: The instruction's hexadecimal dump.
	 * @return: True, iff, the instruction is a direct CALL. False otherwise.
	 */
	
	return (dump[0] != 'F' && dump[0] != 'f');
}

string reverseByteOrder(string const& bytes) {
	/**
	 * Reverse the byte order of a string that represents an hexadecimal byte
	 * flow.
	 * 
	 * @bytes: String that represents the bytes in hexadecimal notation.
	 * @return: String that represents the same bytes of the input string but
	 * in reverse order.
	 */
	 
	string result;
	result.reserve(bytes.size());
	
	for (size_t i = bytes.size(); i != 0; i -= 2)
		result.append(bytes, i-2, 2);
	
	return result;
}

callOpcode getOpcodeCode(string const& opcode) {	
	/**
	 * Get the CALL instruction opcode code.
	 * 
	 * @opcode: String that represents the opcode of the CALL instruction.
	 * @return: CALL opcode code.
	 */
	
	if (opcode == "e8" || opcode == "E8")
		return opE8;
	if (opcode == "9a" || opcode == "9A")
		return op9A;
	else
		return opFF;
}

string getOpcodeString(callOpcode code) {
	/**
	 * Get the CALL instruction opcode string.
	 * 
	 * @opcode: Code that represents the opcode of the CALL instruction.
	 * @return: CALL opcode string.
	 */
	 
	if (code == opE8)
		return "e8";
	if (code == op9A)
		return "9a";
	else
		return "ff";
}

BOOL isAddrExecutable(ADDRINT address) {
	/**
	 * Check if a particular memory address is in an executable page.
	 * 
	 * @address: Address to check.
	 * @return: True if, and only if, @address is in an executable memory page.
	 */
	
	// Find Image to which address belongs.
	PIN_LockClient();
	IMG addrImg = IMG_FindByAddress(address);
	PIN_UnlockClient();
	
	// Find Section to which address belongs.
	SEC sec = IMG_SecHead(addrImg);
	SEC nextSec = SEC_Next(sec);
	while (SEC_Valid(nextSec) && SEC_Address(nextSec) <=  address) {
		sec = nextSec;
		nextSec = SEC_Next(sec);
	}
	
	return SEC_IsExecutable(sec);
}

long getCallTarget(const CONTEXT *ctxt, ADDRINT addr, string dump) {
	/**
	 * Return the target address of a CALL instruction.
	 *
	 * @ctxt: Pointer to CPU context Pin object.
	 * @addr: The instruction's address.
	 * @dump: The instruction's hexadecimal dump.
	 * @return: Target address of input CALL instruction.
	 */
	 
	string opcode = dump.substr(0, 2); // CALL opcode
	string operand; // Operand hex string for some CALL types
	
	unsigned short operandSize; // Operand size for some CALL types
	long IP; // Instruction pointer value
	
	unsigned short modRM; // ModR/M byte for FF CALLs.
	unsigned short mod; // Mod field from ModR/M byte.
	unsigned short reg; // Reg field from ModR/M byte.
	unsigned short RM; // R/M field from ModR/M byte.
	
	unsigned short sib; // SIB byte for FF CALLs.
	unsigned short ss; // Scale field from sib byte.
	unsigned short index; // Index field from sib byte.
	unsigned short base = 8; // Base field from sib byte.
	
	long disp; // Displacement
	ADDRINT regVal = 0; // Holds temporary values obtained from registers.
	
	long target = 0; // CALL target.
	unsigned long memOp = 0; // Holds value read from memory.
	unsigned short memOpSize;//Holds size (in bytes) of value read from memory.
	
	operand = reverseByteOrder(dump.substr(2));
	
	switch (getOpcodeCode(opcode)) {
	case opE8: // Call near, relative
		operandSize = (dump.substr(2).length()) * 4; // Operand size in bits
		IP = addr;
		IP += (dump.length()/2);
		disp = hexToInt(operand);
		
		switch (operandSize) {
		case 64:
		case 32:
			target = IP + disp;	
			break;
		case 16:
			target = ((IP + disp) & 0x0000FFFF);
			break;
		default:
			target = -1;
		}
		
		break;
		
	case op9A: // Call far, absolute
		operand = reverseByteOrder(dump.substr(6));
		target = hexToInt(operand);
		break;
		
	case opFF: // Call near, absolute indirect OR Call far, absolute indirect.
		// ModR/M Byte: Mod (2bit) | Reg (3bit) | R/M (3bit)
		modRM = hexToInt(dump.substr(2, 2));
		
		// Get individual fields.
		mod = (modRM & 0xC0) >> 6;
		reg = (modRM & 0x38) >> 3;
		RM = (modRM & 0x7);
		
		// Select register to use
		switch (RM) {
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
			if (mod < 3) {
				/**
				 * Depends on following SIB (Scale, index, base)
				 * byte.
				 */
				 
				// SIB byte: Scale (2bit) | Index (3bit) | Base (3bit)
				sib = hexToInt(dump.substr(4, 2));
				
				// Get individual fields.
				ss = (sib & 0xC0) >> 6;
				ss = 1 << ss;
				index = (sib & 0x38) >> 3;
				base = (sib & 0x7);
				
				// Calculate base address
				switch (base) {
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
					regVal = PIN_GetContextReg(ctxt, REG_ESP);
					break;
				case 5:
					if (mod != 0)			
						regVal = PIN_GetContextReg(ctxt, REG_EBP);
					break;
				case 6:
					regVal = PIN_GetContextReg(ctxt, REG_ESI);
					break;
				case 7:
					regVal = PIN_GetContextReg(ctxt, REG_EDI);
					break;
				}
				
				// Calculate scaled index.
				switch (index) {
				case 0:
					regVal += PIN_GetContextReg(ctxt, REG_EAX) * ss;
					break;
				case 1:
					regVal += PIN_GetContextReg(ctxt, REG_ECX) * ss;
					break;
				case 2:
					regVal += PIN_GetContextReg(ctxt, REG_EDX) * ss;
					break;
				case 3:
					regVal += PIN_GetContextReg(ctxt, REG_EBX) * ss;
					break;
				case 4:
					break;
				case 5:
					regVal += PIN_GetContextReg(ctxt, REG_EBP) * ss;
					break;
				case 6:
					regVal += PIN_GetContextReg(ctxt, REG_ESI) * ss;
					break;
				case 7:
					regVal += PIN_GetContextReg(ctxt, REG_EDI) * ss;
					break;
				}
			} else {
				regVal = PIN_GetContextReg(ctxt, REG_ESP);
			}
			break;
		case 5:
			if (mod > 0) {
				regVal = PIN_GetContextReg(ctxt, REG_EBP);
			} else {
				regVal = 0;
			}
			break;
		case 6:
			regVal = PIN_GetContextReg(ctxt, REG_ESI);
			break;
		case 7:
			regVal = PIN_GetContextReg(ctxt, REG_EDI);
			break;
		}
		
		// Calculate displacement
		if (RM == 4 && base == 5) {
			disp = hexToInt(reverseByteOrder(dump.substr(6)));
		} else if (mod == 1 || mod == 2 || (mod == 0 && RM == 5)) {
			disp = hexToInt(reverseByteOrder(dump.substr(4)));
		} else {
			disp = 0;
		}
		
		// Read from memory
		if (mod < 3) {
			memOpSize = reg * 2; // Empirical. DWORD, FWORD based on reg value.
			regVal += (ADDRINT) disp;
			PIN_SafeCopy(&memOp, (ADDRINT *) regVal, memOpSize);
			target = memOp;
		} else {
			target = (unsigned long) regVal;
		}
		
		break;
	default:
		target = -1;
	}
	
	return target;
}

bool isCallValid(const CONTEXT *ctxt, ADDRINT addr, string dump) {
	/**
	 * Check if a particular CALL instruction is valid or not.
	 * 
	 * - Indirect Calls are considered valid iff its address is in the last
	 * written position of the LBR. It means this was the last call executed.
	 * - Direct Calls are considered valid iff their target address is
	 * executable.
	 *
	 * @ctxt: Pointer to CPU context Pin object.
	 * @addr: The instruction's address.
	 * @dump: The instruction's hexadecimal dump.
	 */

	if (isDirectCall(dump)) { // Direct CALLs
		TLBCounter++;
		ADDRINT target = getCallTarget(ctxt, addr, dump);

		if ((TLBCounter % TLBmissInterval) == 0) { // TLB miss rate
			TLBCounter = 0;

			bool targetIsExecutable = isAddrExecutable(target);
			return targetIsExecutable;
		} else {
			return true;
		}
	} else { //Indirect CALLs
		/**
		 * Removed LBR call match checking.
		 */
		return true;
	}
}

bool checkFFCallSize(string modRM, unsigned int size) {
	/**
	 * Checks if a particular FF opcode CALL instruction has a valid
	 * size according to its modRM byte.
	 *
	 * @modRM: string that represents the modRM hex byte.
	 * @size: Candidate size.
	 *
	 * @return: True iff the candidate size matches the modRM byte.
	 */
	
	char first = modRM[0], second = modRM[1];
	
	switch(first) {
	case '1':
		if (second == '4' || second == 'c')
			return size == 3;
		else if (second == '5' || second == 'd')
			return size == 6;
		else
			return size == 2;
		break;
	case 'd':
		return size == 2;
		break;
	case '5':
		if (second == '4' || second == 'c')
			return size == 4;
		else
			return size == 3;
		break;
	case '9':
		if (second == '4' || second == 'c')
			return size == 7;
		else
			return size == 6;
		break;
	}
	
	return false;
}

VOID doRET(const CONTEXT *ctxt, ADDRINT returnAddr) {
	/**
	 * Pintool analysis function for return instructions.
	 *
	 * @returnAddr: Return address.
	 */
	
	bool precededByCall = false;
	string foundCallOpcode;
	int callSize = 0;
	int index = 0;
	ADDRINT callAddr = 0;
	string callDump;
	bool callIsDirect = false;
	
	retCount++;
	
	// Get previous 7 bytes (largest CALL size) to return address.
	UINT64 previousBytes = 0;
	PIN_SafeCopy(&previousBytes, (ADDRINT*)(returnAddr - 7), 7);
	
	// Convert to hex string.
	stringstream stream;
	stream << hex << previousBytes;
	string byteString(stream.str());
	byteString.insert(byteString.begin(), 14 - byteString.length(), '0');
	byteString = reverseByteOrder(byteString);
	
	// Check if there is a CALL instruction immediately before this RET
	// instruction.
	for (int size = 2; size <= 7; size++) {
		for (int opcode = opE8; opcode <= opFF; opcode++) {
			if (sizeToOpcodes[size][opcode] == 1) {
				string opcodeStr = getOpcodeString((callOpcode)opcode);
				index = 14 - 2*size;
				callSize = size;
	
				if (byteString.substr(index, 2) == opcodeStr) {
					foundCallOpcode = string(opcodeStr);
					
					if (foundCallOpcode == "ff") {
						precededByCall = \
							checkFFCallSize(byteString.substr(index+2), size); 
					} else {
						precededByCall = true;
						break;
					}
				}
			}
		}
		
		if (precededByCall) {
			callDump = string(byteString.substr(index));
			break;
		}
	}
	
	// Increment proper counters.
	if (!precededByCall) {
		retsNotPrecededByCALLs++;
	} else {
		callAddr = returnAddr - callSize;
		callIsDirect = isDirectCall(callDump);
		
		if (isCallValid(ctxt, callAddr, callDump)) {
			if (callIsDirect)
				retsPrecededByValidDCALL++;
			else
				retsPrecededByValidICALL++;
		} else {
			if (callIsDirect)
				retsPrecededByInvalidDCALL++;
			else
				retsPrecededByInvalidICALL++;
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
	
	callLBR.put(make_pair(addr, true));
}

VOID doIndirectCALL(ADDRINT addr) {
	/**
	 * Pintool analysis fuction for indirect call instructions.
	 *
	 * @addr: The instruction's address.
	 */
	
	callLBR.put(make_pair(addr, false));
}

VOID PIN_FAST_ANALYSIS_CALL doCount(UINT32 numIns) {
	/**
	 * Pintool analysis function for counting the number of instructions in
	 * each basic block.
	 *
	 * @numIns: Number of instructions in the current basic block.
	 */
	 
	instCount += numIns;
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
		BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR) doCount, \
			IARG_FAST_ANALYSIS_CALL, IARG_UINT32, BBL_NumIns(bbl), IARG_END);	
		
        INS tail = BBL_InsTail(bbl);
		
		if (INS_IsRet(tail)) {
			INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR) doRET, \
				IARG_CONTEXT, IARG_BRANCH_TARGET_ADDR, IARG_END);
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
	 * Print the report for the Valid Calls experiment.
	 */
	
	outputFile << dec;

	outputFile << "Instructions:" << instCount << endl;
	outputFile << "RET instructions:" << retCount << endl;
}

VOID Fini(INT32 code, VOID *v) {
	/**
	 * Perform necessary operations when the instrumented application is about
	 * to end execution.
	 */
	 
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
    TLBmissInterval = (int) (1 / (TLBmissRateKnob.Value()/100));

    TRACE_AddInstrumentFunction(InstrumentCode, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}
