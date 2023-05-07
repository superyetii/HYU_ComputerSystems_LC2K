/* LC-2K Instruction-level simulator */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

//Enum and function for opcode types
typedef enum {
    OP_ADD,
    OP_NOR,
    OP_LW,
    OP_SW,
    OP_BEQ,
    OP_JALR,
    OP_HALT,
    OP_NOOP,
    OP_INVALID
} OpCode;

OpCode intToOpCode(int opcode) {
    switch (opcode) {
    case 0: return OP_ADD;
    case 1: return OP_NOR;
    case 2: return OP_LW;
    case 3: return OP_SW;
    case 4: return OP_BEQ;
    case 5: return OP_JALR;
    case 6: return OP_HALT;
    case 7: return OP_NOOP;
    default: return OP_INVALID;
    }
}

//Function for extracting Opcode, arg0, arg1, arg2 instruction from the machine code
void extractInstructionFromMemory(stateType*);
//Function for processing extracted instruction
int processInstruction(stateType*, int, int, int, int);
//Function to call extraction + processing
void executeInstruction();

void printState(stateType *);
int convertNum(int num);

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    for (int i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    extractInstructionFromMemory(&state);
    printState(&state);
    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
	/* convert a 16-bit number into a 32-bit Linux integer */
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return (num);
}

void extractInstructionFromMemory(stateType* statePtr) 
{
    int halt = 0; //Boolean for halting, initialized as false
    int numberOfInstructions = 0; //For counting number of instructions executed.
    while (!halt) {
        //Check PC to confirm which instruction to process
        int counter = statePtr->pc; // PC
        int machineCode = statePtr->mem[counter]; //Brings the instruction we have to run
        int opcode, arg0, arg1, arg2; //Temporary storage for the instructions as char args
        //The following are bit extraction
        opcode = (machineCode >> 22) & 0x3F; //25~22 bit
        arg0 = (machineCode >> 19) & 0x7;    //21~19 bit
        arg1 = (machineCode >> 16) & 0x7;    //18~16 bit
        arg2 = machineCode & 0xFFFF;         //15~0  bit

        halt = processInstruction(statePtr, opcode, arg0, arg1, arg2);
        numberOfInstructions++;
    }
    printf("machine halted\n");
    printf("total of %d instructions executed\n", numberOfInstructions);
    printf("final state of machine:\n");

}

int processInstruction(stateType* statePtr, int opcode, int arg0, int arg1, int arg2) 
{
    printState(statePtr);
    int halt = 0; //Boolean for halting, initialized as false

    OpCode op = intToOpCode(opcode);
    switch (op) {
    case OP_ADD:
        statePtr->reg[arg2] = statePtr->reg[arg0] + statePtr->reg[arg1]; //Add contents of regA with contents of regB, store results in destReg
        statePtr->pc += 1;
        break;
    case OP_NOR:
        statePtr->reg[arg2] = ~(statePtr->reg[arg0] | statePtr->reg[arg1]); //Nor contents of regA with contents of regB, store results in destReg.
        statePtr->pc += 1;
        break;
    case OP_LW:{
        arg2 = convertNum(arg2);
        int memoryAddress = statePtr->reg[arg0] + arg2;     //Calculate memory address by adding regA with offsetField
        statePtr->reg[arg1] = statePtr->mem[memoryAddress]; //Load contents of memory address to regB
        statePtr->pc += 1;
        break;
    }
    case OP_SW: {
        arg2 = convertNum(arg2);
        int memoryAddress = statePtr->reg[arg0] + arg2;     //Calculate memory address by adding regA with offsetField
        statePtr->mem[memoryAddress] = statePtr->reg[arg1]; //Store contents of regB to memory
        statePtr->pc += 1;
        break;
    }
    case OP_BEQ:
        arg2 = convertNum(arg2);
        if (statePtr->reg[arg0] == statePtr->reg[arg1]) {   //Compare contents of regA and regB
            statePtr->pc = statePtr->pc + 1 + arg2;         //Branch with offsetField if true. brancAddress = (PC+1+offsetField)
        }
        else {
            statePtr->pc += 1;                              //Else increment PC by one
        }
        break;
    case OP_JALR: {
        int newPC = statePtr->reg[arg0];  // Address contained in regA
        statePtr->reg[arg1] = statePtr->pc + 1;  // Store PC+1 into regB
        statePtr->pc = newPC;  // Branch to the address contained in regA
        break;
    }
    case OP_HALT:
        statePtr->pc += 1; 
        halt = 1; //Turns halt flag to true
        break;
    case OP_NOOP:
        statePtr->pc += 1; //Nothing to do, just increment program counter
        break;
    case OP_INVALID:
    default:
        exit(1); //Exit if invalid Opcode
    }
    return halt; //returns 1 if halted
}
