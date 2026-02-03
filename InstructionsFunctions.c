#include "Assembler.h"

/*******************************************************************************
 * Processes assembly instructions and encodes them into machine code.
 * Handles instruction encoding, operand processing, and error checking.
 *
 * Parameters:
 * - line: The line containing the instruction to be processed.
 * - table: Pointer to the LabelTable for label management.
 * - Code: Array to hold encoded instructions.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void EncodeInstruction(char *line, LabelTable *table, signed short Code[], int PC[], int line_count, int *Error){
    char *inst; /* Pointer to store the instruction mnemonic */
    char *line_rest; /* Pointer to the rest of the instruction line after the mnemonic */
    int opcode; /* Variable to store the opcode */
    int numOprnd; /* Variable to store the number of operands the instruction expects */
    
    /* Extract the instruction mnemonic and determine its opcode */
    inst = deleteSpaces(strtok(line, " \r\n"));
    opcode = getOpcode(inst);
    
    /* Check if the opcode is valid */
    if(opcode == -1){
        fprintf(stderr, "Error at Line %d, Invalid Instruction %s\n", line_count, inst);
        *Error = 1;
        return;
    }

    /* Encode the opcode in the Code array */
    insertBin((opcode << 6), Code, PC[0]);

    /* Determine the number of operands required by the instruction */
    numOprnd = getNumOperand(opcode);

    /* If the instruction requires no operands, simply increment the program counter and return */
    if(numOprnd == 0){
        PC[0] += 1;
        return;
    }

    /* Extract the rest of the line to process the operands */
    line_rest = deleteSpaces(strtok(NULL, "\r\n"));

    /* Validate the syntax of the instruction line */
    if(!IsValidInstSyntax(line_rest, numOprnd, line_count)){
        *Error = 1;
        return;
    }

    /* Encode the operands */
    EncodeOperands(numOprnd, line_rest, table, Code, opcode, PC, line_count, Error);
}

/*******************************************************************************
 * Returns the number of operands expected by a given opcode.
 *
 * Parameters:
 * - opcode: The opcode for which to determine the number of operands.
 *
 * Returns:
 * - The number of operands required by the opcode.
 ******************************************************************************/
int getNumOperand(int opcode){
    /* Special cases for opcodes that require no operands */
    if(opcode == 14 || opcode == 15) return 0;
    /* Specific opcodes that require two operands */
    if(opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4) return 2;
    /* All other cases are assumed to require one operand */
    return 1;
}


/*******************************************************************************
 * Encodes the operands of an assembly instruction.
 *
 * Parameters:
 * - numOprnd: The number of operands the instruction expects.
 * - line: The line containing the operands.
 * - table: Pointer to the LabelTable for label management.
 * - Code: Array to hold encoded instructions.
 * - opcode: The opcode of the instruction.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Encoding_Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void EncodeOperands(int numOprnd, char* line, LabelTable *table, signed short Code[], int opcode, int PC[], int line_count, int *Encoding_Error) 
{
    char *operand1 = NULL, *operand2 = NULL, *operand = NULL; /* Pointers for operands */
    int addressingMode, word_count = 1, is_reg = 0, i;
    int IC = PC[0];  /* Instruction counter (IC) */

    /* Parse operands based on the number of operands the instruction expects. */
    if(numOprnd == 1){
        operand1 = deleteSpaces(strtok(line, "\r\n"));
    }
    else if(numOprnd == 2){
        operand1 = deleteSpaces(strtok(line, ",\r\n"));
        operand2 = deleteSpaces(strtok(NULL, "\r\n"));
    }

    /* Process each operand to determine its addressing mode and encode it. */
    for (i = 1; i <= numOprnd; i++){
        operand = (i == 1) ? operand1 : operand2; /* Select the current operand. */

        if (!operand) {
            fprintf(stderr, "Error at Line %d: Missing operand(s).\n", line_count);
            
            return;
        }

        addressingMode = getAddressingMode(operand, table); /* Determine addressing mode. */

        switch (addressingMode) {
            case IMMEDIATE: /* Immediate value handling */
                if(!isValidImmediate(operand, line_count) || !IsValidImmUse(i, opcode, numOprnd, line_count)){
                    *Encoding_Error = 1; 
                    return;
                }
                encodeImmediate(operand+1, Code, IC, &word_count);
                break;
                
            case LABEL: /* Label handling */
                if(!encodeLabelOperand(operand, table, Code, IC, &word_count)){
                    fprintf(stderr, "Error at Line %d: Invalid label operand %s\n", line_count, operand);
                   *Encoding_Error = 1; 
                    return;
                }
                break;

            case MATRIX: /* Matrix handling */
                if(!encodeMatrixOperand(operand, table, Code, IC, &word_count, line_count)){
                    *Encoding_Error = 1;
                    return;
                }
                break;
            
            case REGISTER: /* Register handling */
                /* Validate the register operand */
                if (!isValidReg(operand, line_count) || !IsValidRegUse(operand, i, numOprnd, opcode, line_count)) {
                    *Encoding_Error = 1;
                    return;
                }
                encodeRegisterOperand(operand, Code, i, numOprnd, opcode, IC, &is_reg, &word_count);
                break;
         
            default:
                *Encoding_Error = 1;
                fprintf(stderr, "Error at Line %d: Invalid operand %s\n", line_count, operand);
                return;
        }

        /* Insert addressing mode for operand(s) into the Code array. */
        if (i == 1 && numOprnd == 2) {
            insertBin((addressingMode << 4), Code, IC);  /* For the first of two operands. */
        } else {
            insertBin((addressingMode << 2), Code, IC);  /* For a single operand or the second of two operands. */
        }
    }
    PC[0] += word_count; /* Update the instruction counter (IC) after encoding operands. */
}


/*******************************************************************************
 * Encodes an immediate operand in assembly instructions.
 *
 * Parameters:
 * - operand: The immediate operand to be encoded.
 * - Code: Array to hold encoded instructions.
 * - IC : Instruction counter.
 * - word_count: Pointer to the word count for the current instruction.
 ******************************************************************************/
void encodeImmediate(char *operand, signed short Code[], int IC, int *word_count){
    signed short imm; /* Variable to hold the immediate value */
   
    imm = (signed short)atoi(operand);

    /* Encode the immediate value into the Code array */
    insertBin((imm << 2), Code, (IC + *word_count));
    (*word_count)++; /* Increment the word count */
}


/*******************************************************************************
 * Encodes a label operand in assembly instructions.
 *
 * Parameters:
 * - operand: The label operand to be encoded.
 * - table: Pointer to the LabelTable for label management.
 * - Code: Array to hold encoded instructions.
 * - IC : Instruction counter.
 * - word_count: Pointer to the word count for the current instruction.
 ******************************************************************************/
int encodeLabelOperand(char *operand, LabelTable *table, signed short Code[], int IC, int *word_count){
    int index; /* Index in the label table where the label is expected to be found */
    int A_R_E; /* Variable to hold the Absolute, Relocatable, External attribute */
    Label *current_label; /* Pointer to traverse labels in the table */

    /* Find the label in the table */
    index = findLabel(table, operand);
    /* Ensure the label was found */
    if (index == -1) return 0;
    
    /* Get the current label from the table */
    current_label = table->Labels[index];
    /* Traverse the linked list at the table index to find the exact label */
    while (current_label){
        if(strcmp(current_label->name, operand) == 0){
            /* Check if the label is defined using .mat , which is invalid for a label operand */
            if(current_label->mat) return 0;

            /* Determine A_R_E bits based on whether the label is external or not */
            A_R_E = current_label->ext ? 1 : 2;

            /* Record the reference to the label for later address resolution */
            saveRef(current_label, (IC + *word_count));
            break;
        } 
        current_label = current_label->next;
    }

    /* Encode the A_R_E attribute into the Code array at the specified position */
    insertBin(A_R_E, Code, (IC + *word_count));
    /* Increment the word count after encoding the label operand */
    (*word_count)++;

    return 1; /* Return 1 for successful encoding */
}


/*******************************************************************************
 * Encodes a matrix operand in assembly instructions.
 *
 * Parameters:
 * - matrix: The matrix operand to be encoded.
 * - table: Pointer to the LabelTable for label management.
 * - Code: Array to hold encoded instructions.
 * - IC : Instruction counter.
 * - word_count: Pointer to the word count for the current instruction.
 * - line_count: Current line number for error reporting.
 ******************************************************************************/
int encodeMatrixOperand(char *matrix, LabelTable* table, signed short Code[], int IC, int* word_count, int line_count){
    char mat_name[MAX_LINE_LENGTH];
    unsigned short regs[2];
    int index, A_R_E;
    Label *current_label; /* Pointer to traverse labels in the table */


    if (!ValidateAndParseMatrixOperand(matrix, table, regs, mat_name, line_count)) {
        return 0; 
    }

    index = findLabel(table, mat_name);
    current_label = table->Labels[index];
    /* Traverse the linked list at the table index to find the exact label */
    while (current_label){
        if(strcmp(current_label->name, mat_name) == 0){
            A_R_E =  2;
            /* Record the reference to the label for later address resolution */
            saveRef(current_label, (IC + *word_count));
            break;
        } 
        current_label = current_label->next;
    }

    /* Encode the A_R_E attribute into the Code array at the specified position */
    insertBin(A_R_E, Code, (IC + *word_count));
    (*word_count)++; 

    insertBin((regs[0] << 6), Code, (IC + *word_count));
    insertBin((regs[1] << 2), Code, (IC + *word_count));
    (*word_count)++; /* Increment the word count after encoding the matrix label */
    
    
    return 1; 
}


/*******************************************************************************
 * Encodes a register operand in assembly instructions.
 *
 * Parameters:
 * - operand: The register operand to be encoded.
 * - Code: Array to hold encoded instructions.
 * - i: The index of the operand (1-based).
 * - numOprnd: The number of operands the instruction expects.
 * - opcode: The opcode of the instruction.
 * - IC : Instruction counter.
 * - is_reg: Pointer to a flag indicating if the operand is a register.
 * - word_count: Pointer to the word count for the current instruction.
 ******************************************************************************/
int encodeRegisterOperand(char *operand, signed short Code[], int i, int numOprnd, int opcode, int IC, int *is_reg, int *word_count){
    int reg; /* Variable to store the register number */
    
    /* Convert the operand (skipping 'r' or 'R' prefix) to a number */
    reg = atoi(++operand);

    /* Encoding varies based on the number of operands and their positions */
    if (numOprnd == 1) {
        /* For single-operand instructions, encode the register directly */
        insertBin((reg << 2), Code, (IC + *word_count));
        (*word_count)++;
    } else if (i == 1) {
        /* For the first operand in two-operand instructions, shift left and set is_reg flag */
        insertBin((reg << 6), Code, (IC + *word_count));
        (*word_count)++;
        *is_reg = 1;
    } else if (i == 2) {
        /* For the second operand, encoding depends on whether the first operand was a register */
        if (*is_reg) {
            /* If the first operand was a register, encode both registers in the same word */
            insertBin((reg << 2), Code, (IC + (*word_count - 1)));
            *is_reg = 0;
        } else {
            /* If the first operand wasn't a register, encode the second operand separately */
            insertBin((reg << 2), Code, (IC + *word_count));
            (*word_count)++;
        }
    }

    return 1; /* Register operand encoding successful */
}


/*******************************************************************************
 * Gets the opcode for a given instruction mnemonic.
 *
 * Parameters:
 * - inst: The instruction mnemonic to look up.
 *
 * Returns:
 * - The opcode corresponding to the instruction mnemonic, or -1 if not found.
 ******************************************************************************/
int getOpcode(char *inst){
    /* Check the instruction mnemonic and return the corresponding opcode */
    if(strcmp(inst, "mov") == 0) return 0;
    else if(strcmp(inst, "cmp") == 0) return 1;
    else if(strcmp(inst, "add") == 0) return 2;
    else if(strcmp(inst, "sub") == 0) return 3;
    else if(strcmp(inst, "lea") == 0) return 4;
    else if(strcmp(inst, "clr") == 0) return 5;
    else if(strcmp(inst, "not") == 0) return 6;
    else if(strcmp(inst, "inc") == 0) return 7;
    else if(strcmp(inst, "dec") == 0) return 8;
    else if(strcmp(inst, "jmp") == 0) return 9;
    else if(strcmp(inst, "bne") == 0) return 10;
    else if(strcmp(inst, "jsr") == 0) return 11;
    else if(strcmp(inst, "red") == 0) return 12;
    else if(strcmp(inst, "prn") == 0) return 13;
    else if(strcmp(inst, "rts") == 0) return 14;
    else if(strcmp(inst, "stop") == 0) return 15;
    else return -1;
}


/*******************************************************************************
 * Inserts a binary representation of a signed short integer into the Code array.
 *
 * Parameters:
 * - x: The signed short integer to be inserted.
 * - Code: Array to hold encoded instructions.
 * - count: The index in the Code array where the binary representation should be inserted.
 ******************************************************************************/
void insertBin(signed short x, signed short Code[], int count){
    int bit, i;
    unsigned short tmp = (unsigned short)x;
    /* Iterate through each bit of the integer */
    for(i = 0; i < SIZE_OF_BITS; i++){
        bit = (tmp % 2);
        Code[count] |= bit << i; /* Set the corresponding bit in the Code array */
        tmp >>= 1; /* Shift to the next bit */
    }
}


/*******************************************************************************
 * Saves a reference to a label's position in the instruction code.
 *
 * Parameters:
 * - label: Pointer to the label to save the reference for.
 * - address: The address in the instruction code to reference.
 ******************************************************************************/
void saveRef(Label *label, unsigned short address) {
    /* Allocate memory for a new reference */
    Reference* ref = (Reference*)malloc(sizeof(Reference));
    ref->next = label->ref; /* Insert the new reference at the beginning of the list */
    label->ref = ref;
    label->ref->pos = address; /* Set the reference's position */
}


/*******************************************************************************
 * Gets the addressing mode for a given operand.
 *
 * Parameters:
 * - operand: The operand to analyze.
 * - table: Pointer to the LabelTable for label management.
 *
 * Returns:
 * - The addressing mode corresponding to the operand, or -1 if not found.
 ******************************************************************************/
int getAddressingMode(char* operand, LabelTable* table){
    int addressing = -1; /* Default to an unrecognized addressing mode */
    char *first_bracket, *second_bracket;

    /* Check for immediate addressing mode */
    if(*operand == '#'){
        addressing = 0; 
    }
    /* Direct addressing mode */
    else if(findLabel(table, operand) != -1){
        addressing = 1;
    }
    /* Matrix  addressing mode */
    else if ((first_bracket = strchr(operand, '[')) && (second_bracket = strchr(operand, ']')) && first_bracket < second_bracket){
        if(strchr(second_bracket, '[') && strchr(second_bracket+1, ']')){
            addressing = 2;
        }
    }
    /* Register addressing mode */
    else if(*operand == 'r'){
        addressing = 3;
    }
    return addressing; /* Return the determined addressing mode */
}

