/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

/*Storing Labels*/
//In the first pass, it will calculate the address for every symbolic label. Assume that the first instruction is at address 0.
//Source: Project_1.pdf --> 4. LC-2K Assembly Language and Assembler (40pts)
typedef struct Label
{
	char label_name[10]; //Note: Valid labels contain a maximum of 6 characters and can consist of letters and numbers
	int label_value; //Address field for the Label, begins with 0, and increments by 1. Also may contain value in case from .fill
	struct Label* next; //Points to the next Label in line.
	//Note: This will require helper functions to traverse and find a specific label

} Label;

Label* label_list = NULL; //Head of the labels, initialize as NULL

//Base functions given in skeleton code
int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

//Function for creating Label Nodes
int processLabels(FILE*, char*, char*, char*, char*, char*);

//Function for converting Instruction to Machine Code
int toMachineCode(FILE*, FILE*, char*, char*, char*, char*, char*);

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
	OP_FILL,
	OP_INVALID
} OpCode;

OpCode stringToOpCode(const char* opcode) {
	if (strcmp(opcode, "add") == 0) return OP_ADD;
	if (strcmp(opcode, "nor") == 0) return OP_NOR;
	if (strcmp(opcode, "lw") == 0) return OP_LW;
	if (strcmp(opcode, "sw") == 0) return OP_SW;
	if (strcmp(opcode, "beq") == 0) return OP_BEQ;
	if (strcmp(opcode, "jalr") == 0) return OP_JALR;
	if (strcmp(opcode, "halt") == 0) return OP_HALT;
	if (strcmp(opcode, "noop") == 0) return OP_NOOP;
	if (strcmp(opcode, "noop") == 0) return OP_NOOP;
	if (strcmp(opcode, ".fill") == 0) return OP_FILL;
	return OP_INVALID;
}

/*
Error Checking
Your assembler should catch the following errors in the assembly-language program:
-Use of undefined labels                   (Clear) //Applied individually in toMachineCode() function
-Duplicate definition of labels            (Clear) //Separate Function called before toMachineCode invoke 
-offsetFields that don¡¯t fit in 16 bits    (Clear) //Separate Function called within toMachineCode function
-Unrecognized opcodes                      (Clear) //Considered within toMachineCode function through enums
-Non-integer register arguments            (Clear) //Separate Function called within toMachineCode function
-Registers outside the range [0, 7]        (Clear) //Separate Function called within toMachineCode function
Your assembler should exit(1) if it detects an error and exit(0) if it finishes without detecting any
errors. Your assembler should NOT catch simulation-time errors, i.e. errors that would occur at the time the
assembly-language program executes (e.g. branching to address -1, infinite loops, etc.).
*/

//Error checking functions
int noDuplicateLabels(Label*); //checks duplicate labels
int isWithinRange(char*);      //checks offsetField within range of -32768 to 32767
int regintRange(char*);        //checks register 1.) Is integer 2.) within 0 ~ 7


int main(int argc, char *argv[]) 
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr;
	char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH], 
			 arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

	//Checks if the program has been executed with both input and output file name
	if (argc != 3) {
		printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
				argv[0]);
		exit(1);
	}

	inFileString = argv[1];
	outFileString = argv[2];

	inFilePtr = fopen(inFileString, "r");
	if (inFilePtr == NULL) {
		printf("error in opening %s\n", inFileString);
		exit(1);
	}
	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}


	//Phase-1 for Label Calculation
	if (processLabels(inFilePtr, label, opcode, arg0, arg1, arg2) != 1) {
		exit(1);
	}
	/* this is how to rewind the file ptr so that you start reading from the
		 beginning of the file */
	rewind(inFilePtr);

	if (noDuplicateLabels(label_list)) { //returns 1 if a duplicate has been found
		exit(1);
	}

	if (toMachineCode(inFilePtr, outFilePtr, label, opcode, arg0, arg1, arg2) != 1) {
		exit(1);
	}

	if (inFilePtr) {
		fclose(inFilePtr);
	}
	if (outFilePtr) {
		fclose(outFilePtr);
	}
	return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
		char *arg1, char *arg2)
{
	char line[MAXLINELENGTH];
	char *ptr = line;

	/* delete prior values */
	label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

	/* read the line from the assembly-language file */
	if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		/* reached end of file */
		return(0);
	}

	/* check for line too long (by looking for a \n) */
	if (strchr(line, '\n') == NULL) {
		/* line too long */
		printf("error: line too long\n");
		exit(1);
	}

	/* is there a label? */
	ptr = line;
	if (sscanf(ptr, "%[^\t\n\r ]", label)) {
		/* successfully read label; advance pointer over the label */
		ptr += strlen(label);
	}

	/*
	 * Parse the rest of the line.  Would be nice to have real regular
	 * expressions, but scanf will suffice.
	 */
	sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
			"[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
	return(1);
}

int isNumber(char *string)
{
	/* return 1 if string is a number */
	int i;
	return( (sscanf(string, "%d", &i)) == 1);
}

//Implementation of processLabel function
int processLabels(FILE* inFilePtr, char* label, char* opcode, char* arg0,
	char* arg1, char* arg2)
{
	int line = 0;
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
		if (strlen(label) > 0) { //checks if label is empty or not
			Label* new_label = (Label*)malloc(sizeof(Label));
			strcpy(new_label->label_name, label);
			new_label->label_value = line;
			new_label->next = label_list;
			label_list = new_label;

			//Since it is label, we also check whether this is a .fill
			if (!strcmp(opcode, ".fill")) {
				if (isNumber(arg0)) { //Is number, directly add value
					new_label->label_value = atoi(arg0); //Adds to the value section regardless of type
				}
				else { //Not a number, find the label and add label value
					Label* currentNode = label_list; //Match the currentNode to the head Node of labels
					while (strcmp(currentNode->label_name, arg0)) { //Look for matching label name
						currentNode = currentNode->next;
						if (currentNode == label_list) { //Came back to head without finding the label
							exit(1); //Exiting with error : Label cannot be found
						}
					}
					new_label->label_value, currentNode->label_value;
				}
			}
		}
		line++;
	}
	return 1; //Success
}

//Implementation of toMachineCode function
int toMachineCode(FILE* inFilePtr, FILE* outFilePtr, char* label, char* opcode, char* arg0,
	char* arg1, char* arg2)
{
	int line = 0; //Temporary variable to count PC
	int machineCode; //Temporary location for machine code before writing to output file.
	while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
		//Note: Total of 9 possible cases for opcode (or 10 if we include invalid)
		//add, nor, lw, sw, beq, jalr, halt, noop, .fill
		OpCode op = stringToOpCode(opcode);
		switch (op) {
		case OP_ADD:
			if (!(regintRange(arg0) && regintRange(arg1))) exit(1);
			machineCode = (0 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2) & 0xFFFF);
			break;
		case OP_NOR:
			if (!(regintRange(arg0) && regintRange(arg1))) exit(1);
			machineCode = (1 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2) & 0xFFFF);
			break;

		//Note: According to the project1 pdf, there are only three cases where there can be sympolic addresses
		//These are the I-type instructions that are namely LW, SW, BEQ
		//For LW and SW, we simply find the value associatd with the label
		//For BEQ we calculate the offsetField using "offsetField = target_address - (PC + 1)"
		case OP_LW:
			if (!isNumber(arg2)) {  //Happens only when if we are looking for a label
				Label* currentNode = label_list; //Match the currentNode to the head Node of labels
				while (strcmp(currentNode->label_name, arg2)) {
					currentNode = currentNode->next;
					if (currentNode == label_list) { //Came back to head without finding the label
						exit(1); //Exiting with error : Label cannot be found
					}
				}
				snprintf(arg2, sizeof(arg2), "%d", currentNode->label_value);//Bring value from label to arg2
			}
			if (!isWithinRange(arg2)) {
				exit(1);
			}
			if (!(regintRange(arg0) && regintRange(arg1))) exit(1);
			machineCode = (2 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2) & 0xFFFF);
			break;
		case OP_SW:
			if (!isNumber(arg2)) {  //Happens only when if we are looking for a label
				Label* currentNode = label_list; //Match the currentNode to the head Node of labels
				while (strcmp(currentNode->label_name, arg2)) {
					currentNode = currentNode->next;
					if (currentNode == label_list) { //Came back to head without finding the label
						exit(1); //Exiting with error : Label cannot be found
					}
				}
				snprintf(arg2, sizeof(arg2), "%d", currentNode->label_value);//Bring value from label to arg2
			}
			if (!isWithinRange(arg2)) {
				exit(1);
			}
			if (!(regintRange(arg0) && regintRange(arg1))) exit(1);
			machineCode = (3 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2) & 0xFFFF);
			break;
		case OP_BEQ:
			if (!isNumber(arg2)) {  //Happens only when if we are looking for a label
				Label* currentNode = label_list; //Match the currentNode to the head Node of labels
				while (strcmp(currentNode->label_name, arg2)) {
					currentNode = currentNode->next;
					if (currentNode == label_list) { //Came back to head without finding the label
						exit(1); //Exiting with error : Label cannot be found
					}
				}
				int offsetField = (currentNode->label_value) - (line + 1);/*Calculate relative offsetField*/
				snprintf(arg2, sizeof(arg2), "%d", offsetField); //inserts the new offsetField into arg2
			}
			if (!isWithinRange(arg2)) {
				exit(1);
			}
			if (!(regintRange(arg0) && regintRange(arg1))) exit(1);
			machineCode = (4 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2) & 0xFFFF);
			break;
		case OP_JALR:
			machineCode = (5 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16);
			break;
		case OP_HALT:
			machineCode = (6 << 22);
			break;
		case OP_NOOP:
			machineCode = (7 << 22);
			break;
		case OP_FILL:
			if (!isNumber(arg0)) {
				Label* currentNode = label_list; //Match the currentNode to the head Node of labels
				while (strcmp(currentNode->label_name, arg0)) {
					currentNode = currentNode->next;
					if (currentNode == label_list) { //Came back to head without finding the label
						exit(1); //Exiting with error : Label cannot be found
					}
				}
				snprintf(arg0, sizeof(arg0), "%d", currentNode->label_value); //inserts the new value to arg0
			}
			machineCode = atoi(arg0);
			break;
		case OP_INVALID:
		default:
			// handle invalid or unknown opcode
			exit(1);
			break;
		}
		// Write the machine code instruction to the output file in hex form
		fprintf(outFilePtr, "%d\n", machineCode);
		line++;
	
	}

	return 1;
}

//Error checking functions
int noDuplicateLabels(Label* label_list) {
	Label* current = label_list;
	Label* compare;
	while (current != NULL) {
		compare = current->next;

		while (compare != NULL) {
			if (strcmp(current->label_name, compare->label_name) == 0) {
				// Found a duplicate label name
				return 1;
			}
			compare = compare->next;
		}

		current = current->next;
	}

	// No duplicate labels found
	return 0;
}

int isWithinRange(char* arg) {
	int num = atoi(arg);
	if (num >= -32768 && num <= 32767) {
		return 1; //valid
	}
	else {
		return 0;
	}
}

int regintRange(char* arg) {
	if (isNumber(arg) && ((atoi(arg) >= 0) && (atoi(arg) <= 7))) {
		return 1; //valid
	}
	else {
		return 0; //invalid
	}
}
