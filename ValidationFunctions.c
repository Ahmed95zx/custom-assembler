#include "Assembler.h"

/*******************************************************************************
 * Validates a macro name against reserved words, register names, and instruction mnemonics.
 * Ensures the macro name does not conflict with any of these.
 *
 * Parameters:
 * - macro_name: The name of the macro to validate.
 * - counter: Pointer to an integer for error reporting line number.
 *
 * Returns:
 * - 1 if the macro name is valid, 0 otherwise.
 ******************************************************************************/
int IsValidMacroName(char* macro_name, int* counter){
    const char *reserved_word[] = {"string", "data", "entry", "extern"};
    const char *register_name[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};
    int i;
    
    for (i = 0; i < sizeof(reserved_word) / sizeof(reserved_word[0]); i++) {
        if (strcmp(macro_name, reserved_word[i]) == 0) {
            fprintf(stderr, "Error at line %d: Macro name has been set as a reserved word: %s\n", *counter, macro_name);
            return 0;
        }
    }

    for (i = 0; i < sizeof(register_name) / sizeof(register_name[0]); i++) {
        if (strcmp(macro_name, register_name[i]) == 0){
            fprintf(stderr, "Error at line %d: Macro name has been set as a register name: %s\n", *counter, macro_name);
            return 0;
        }
    }

    if (getOpcode(macro_name) != -1) {
        fprintf(stderr, "Error at line %d: Macro name matches an instruction: %s\n", *counter, macro_name);
        return 0;
    }
    return 1;
}


/*******************************************************************************
 * Validates a label name against reserved words, register names, and instruction mnemonics.
 * Ensures the label name does not conflict with any of these.
 *
 * Parameters:
 * - label_name: The name of the label to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the label name is valid, 0 otherwise.
 ******************************************************************************/
int validLabel(char *label_name, int line_count){
    int i, len;
    const char *reserved_word[] = {"string", "data", "entry", "extern"};
    const char *register_name[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

    if (strchr(label_name, ' ') || strchr(label_name, '\t')) {
        fprintf(stderr, "Error at Line %d: Illegal space in label definition: %s\n", line_count, label_name);
        return 0;
    }

    len = strlen(label_name);
    if (len > MAX_LABEL) {
        fprintf(stderr, "Error at Line %d: Label name is too long, maximum length is %d characters: %s\n", line_count, MAX_LABEL, label_name);
        return 0;
    }

    if (!isalpha(label_name[0])) {
        fprintf(stderr, "Error at Line %d: First character of label name should be a letter: %s\n", line_count, label_name);
        return 0;
    }

    for (i = 1; i < len; i++) {
        if (!isalnum(label_name[i])) {
            fprintf(stderr, "Error at Line %d: Extraneous text at the label name: %s\n", line_count, label_name);
            return 0;
        }
    }

    for (i = 0; i < sizeof(reserved_word) / sizeof(reserved_word[0]); i++) {
        if (strcmp(label_name, reserved_word[i]) == 0) {
            fprintf(stderr, "Error at Line %d: Label name has been set as a reserved word: %s\n", line_count, label_name);
            return 0;
        }
    }

    for (i = 0; i < sizeof(register_name) / sizeof(register_name[0]); i++) {
        if (strcmp(label_name, register_name[i]) == 0) {
            fprintf(stderr, "Error at Line %d: Label name has been set as a register name: %s\n", line_count, label_name);
            return 0;
        }
    }

    if (getOpcode(label_name) != -1) {
        fprintf(stderr, "Error at Line %d: Label name matches an instruction: %s\n", line_count, label_name);
        return 0;
    }

    return 1;
}


/*******************************************************************************
 * Validates the syntax of an instruction line.
 *
 * Parameters:
 * - line: The line of code to validate.
 * - numOprnd: The number of operands expected for the instruction.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the instruction syntax is valid, 0 otherwise.
 ******************************************************************************/
int IsValidInstSyntax(char *line, int numOprnd, int line_count){
    
    /* Check for extraneous text or illegal use of commas */
    if(numOprnd == 0 && line != NULL){
        fprintf(stderr, "Error at Line %d, Extraneous text after end of Instruction\n", line_count);
        return 0;
    }
    
    /* Illegal comma placements */
    if(*line == ',' || line[strlen(line)-1] == ',' ){
        fprintf(stderr, "Error at Line %d, Illegal Comma\n", line_count);
        return 0;
    }
    
    /* One-operand instructions should not have a comma */
    if(numOprnd == 1 && strchr(line, ',')){
        fprintf(stderr, "Error at Line %d, Illegal Comma\n", line_count);
        return 0;
    }

    /* One-operand instructions should not have extra text after the operand */
    if(numOprnd == 1 && (strchr(line, ' ') || strchr(line, '\t'))){
        fprintf(stderr, "Error at Line %d, Extraneous text after end of Instruction\n", line_count);
        return 0;
    }

    /* Two-operand instructions must have a comma */
    if(numOprnd == 2 && !strchr(line, ',')){
        fprintf(stderr, "Error at Line %d, Missing Comma\n", line_count);
        return 0;
    }

    return 1; /* The instruction syntax is valid */
}


/*******************************************************************************
 * Validates the usage of immediate values in instruction operands.
 *
 * Parameters:
 * - i: The operand index (1-based).
 * - opcode: The opcode of the instruction.
 * - numOprnd: The number of operands for the instruction.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the immediate usage is valid, 0 otherwise.
 ******************************************************************************/
int IsValidImmUse(int i, int opcode, int numOprnd , int line_count){
    /* First operand of a two-operand instruction where immediate is not allowed for specific opcodes */
    if (i == 1 && ((numOprnd == 1 && opcode != 13) || (numOprnd == 2 && opcode == 4))) {
        fprintf(stderr, "Error at Line %d: Immediate value not allowed in this position for opcode %d\n", line_count, opcode);
        return 0;
    }
    /* Second operand where immediate is not allowed except for opcode 1 */
    if (i == 2 && opcode != 1) {
        fprintf(stderr, "Error at Line %d: Immediate value not allowed in this position for opcode %d\n", line_count, opcode);
        return 0;
    }

    return 1; /* Immediate usage is valid */
}


/*******************************************************************************
 * Validates the format of an immediate operand.
 *
 * Parameters:
 * - operand: The immediate operand to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the immediate format is valid, 0 otherwise.
 ******************************************************************************/
int isValidImmediate(char *operand, int line_count){

    /* Check if the operand is a valid immediate value */
    if (operand[0] != '#' ) {
        fprintf(stderr, "Error at Line %d: Invalid immediate format: %s\n", line_count, operand);
        return 0;
    }

    operand++; /* Move past the initial '#' */
    if(!isValidNum(operand)){
        fprintf(stderr, "Error at Line %d: Invalid immediate format2: %s\n", line_count, operand);
        return 0;
    }
        
    return 1;
}


/*******************************************************************************
 * Validates a numeric operand.
 *
 * Parameters:
 * - num: The numeric operand to validate.
 *
 * Returns:
 * - 1 if the numeric format is valid, 0 otherwise.
 ******************************************************************************/
int isValidNum(char *num){
        char *c;
        int i;
        for(c = num, i = 0; *c != '\0'; c++, i++) {
            /* Allow an optional leading sign */
            if((*c == '-' || *c == '+') && i == 0) c++;
            /* Ensure the rest of the parameter is digits */
            if(!(isdigit(*c))){
                return 0;
            }
        }
        return 1;
    }


/*******************************************************************************
 * Validates a register operand.
 *
 * Parameters:
 * - operand: The register operand to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the register operand is valid, 0 otherwise.
 ******************************************************************************/
int isValidReg(char *operand, int line_count){
    int reg = -1;

    /* Check if the operand is a valid register */
    if (operand[0] != 'r' || !isdigit(operand[1]) || strlen(operand) != 2) {
        fprintf(stderr, "Error at Line %d: Invalid register format: %s\n", line_count, operand);
        return 0;
    }

    /* Convert the register number to an integer */
    reg = atoi(operand + 1);

    /* Check if the register number is within the valid range (0-7) */
    if (reg < 0 || reg > 7) {
        fprintf(stderr, "Error at Line %d: Invalid register number: %s\n", line_count, operand);
        return 0;
    }

    return 1; 
}


/*******************************************************************************
 * Validates the usage of registers in instruction operands.
 *
 * Parameters:
 * - operand: The register operand to validate.
 * - i: The operand index (1-based).
 * - numOprnd: The number of operands for the instruction.
 * - opcode: The opcode of the instruction.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the register usage is valid, 0 otherwise.
 ******************************************************************************/
int IsValidRegUse(char *operand, int i, int numOprnd, int opcode, int line_count){

    if (i == 1 && numOprnd == 2 && opcode == 4) {
        fprintf(stderr, "Error at Line %d: Register not allowed in this position for opcode %d\n", line_count, opcode);
        return 0;
    }
    return 1; 
}


/*******************************************************************************
 * Validates the format of a matrix operand.
 *
 * Parameters:
 * - operand: The matrix operand to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the matrix format is valid, 0 otherwise.
 ******************************************************************************/
int isLegalBrackets(char *operand) {
    const char *b1, *e1, *b2, *e2, *p;

    if (!operand) return 0;

    /* First [ ... ] */
    b1 = strchr(operand, '[');
    if (!b1) return 0;

    e1 = strchr(b1 + 1, ']');
    if (!e1) return 0;

    /* Second [ ... ] after the first ] */
    b2 = strchr(e1 + 1, '[');
    if (!b2) return 0;

    e2 = strchr(b2 + 1, ']');
    if (!e2) return 0;

    /* Ensure order: b1 < e1 < b2 < e2 */
    if (!(b1 < e1 && e1 < b2 && b2 < e2)) return 0;

    /* No extra brackets after the second closing ] (ignore trailing spaces) */
    p = e2 + 1;
    while (*p && isspace((unsigned char)*p)) p++;
    if (strchr(p, '[') || strchr(p, ']')) return 0;

    return 1;
}


/*******************************************************************************
 * Validates and parses a matrix operand.
 *
 * Parameters:
 * - operand: The matrix operand to validate and parse.
 * - table: Pointer to the label table for label resolution.
 * - regs: Array to store register numbers.
 * - mat_name: Buffer to store the matrix name.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the matrix operand is valid, 0 otherwise.
 ******************************************************************************/
int ValidateAndParseMatrixOperand(char* operand, LabelTable *table, unsigned short regs[], char mat_name[], int line_count){
    char *tmp_name = NULL, *reg = NULL; /* Pointers for parsing the matrix operand */
    Label *current_label = NULL;
    int index = 0, i = 0; /* Index in the label table */
    unsigned short reg_num = 0;
    /* Check for legal brackets */
    if(!isLegalBrackets(operand)) {
        fprintf(stderr, "Error at Line %d: Unmatched brackets in matrix operand: %s\n", line_count, operand);
        return 0;
    }

    /* Extract the matrix name */
    tmp_name = deleteSpaces(strtok(operand, "["));
    if(!tmp_name || *tmp_name == '\0') {
        fprintf(stderr, "Error at Line %d: Missing matrix name in operand\n", line_count);
        return 0;
    }

    /* Find the matrix label in the label table */
    if((index = findLabel(table, tmp_name)) == -1){
        fprintf(stderr, "Error at Line %d: matrix %s not found\n", line_count, tmp_name);
        return 0;
    }
    current_label = table->Labels[index];
    while (current_label != NULL){
        if (strcmp(current_label->name, tmp_name) == 0) {
            if(current_label->mat == 0) {
                /* Not a matrix */
                fprintf(stderr, "Error at Line %d: label %s is not a Matrix\n", line_count, tmp_name);
                return 0;
            }
        }
        current_label = current_label->next;
    }

    /* Copy the matrix name to the provided buffer */
    strcpy(mat_name, tmp_name);

    /* Extract the register parts */
    reg = deleteSpaces(strtok(NULL, "]"));
    for(i = 0; i <= 1; i++){
        if(!reg || *reg == '\0') {
            fprintf(stderr, "Error at Line %d: Missing register in matrix operand\n", line_count);
            return 0;
        }
        if(!isValidReg(reg, line_count)) {
            fprintf(stderr, "Error at Line %d: Invalid register in matrix operand: %s\n", line_count, reg);
            return 0;
        }
        reg_num = (unsigned short)atoi(reg + 1); /* Convert register string to short int */
        regs[i] = reg_num;

        if(i == 0){
            reg = deleteSpaces(strtok(NULL, "]"));
            if (*reg != '['){
                fprintf(stderr, "Error at Line %d: Illegal character between matrix brackets\n", line_count);
                return 0;
            }
            reg++; 
        }
    }

    /* Check for extra text after the matrix operand */
    if(strtok(NULL, "\r\n")){
        fprintf(stderr, "Error at Line %d: Extra text in matrix operand\n", line_count);
        return 0;
    }

    return 1;
}


/*******************************************************************************
 * Validates the usage of immediate values in instruction operands.
 *
 * Parameters:
 * - i: The operand index (1-based).
 * - opcode: The opcode of the instruction.
 * - numOprnd: The number of operands for the instruction.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the immediate usage is valid, 0 otherwise.
 ******************************************************************************/
int IsValidImmediateUsage(int i, int opcode, int numOprnd, int line_count) {
    /* Check if the immediate value is used in the correct context */
    if (i == 1 && (opcode == 0x01 || opcode == 0x02)) {
        fprintf(stderr, "Error at Line %d: Immediate value cannot be used with opcode %02X\n", line_count, opcode);
        return 0;
    }
    return 1;
}


/*******************************************************************************
 * Validates the syntax of a data line.
 *
 * Parameters:
 * - dataLine: The data line to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the data line syntax is valid, 0 otherwise.
 ******************************************************************************/
int IsValidDataSyntax(char *dataLine, int line_count){
    /* Check for a leading or trailing comma */
    if(*dataLine == ',' || dataLine[strlen(dataLine)-1] == ','){
        fprintf(stderr, "Error at Line %d, Illegal Comma in data line\n", line_count);
        return 0;
    }

    /* Check for missing commas between parameters */
    if(!(strchr(dataLine, ',')) && (strchr(dataLine, ' ') || strchr(dataLine, '\t'))){
        fprintf(stderr, "Error at Line %d, Missing Comma in data line\n", line_count);
        return 0;
    }

    /* Check for double commas */
    if(hasDoubleCommas(dataLine)){
        fprintf(stderr, "Error at Line %d, Double Commas in data line\n", line_count);
        return 0;
    }

    return 1; /* Syntax is valid */
}


/*******************************************************************************
 * Checks for double commas in a line of code.
 *
 * Parameters:
 * - line: The line of code to check.
 *
 * Returns:
 * - 1 if double commas are found, 0 otherwise.
 ******************************************************************************/
int hasDoubleCommas(const char* line) {
    int i; /* Loop counter */
    const char *c = line; /* Pointer to navigate the line */

    /* Iterate through each character in the line */
    for (i = 0; c[i] != '\0'; i++) {
        /* Check for a comma */
        if (c[i] == ',') {
            /* Skip any whitespace following the comma */
            while (c[i + 1] != '\0' && (isspace(c[i + 1]) || c[i + 1] == '\t')) {
                i++;
            }
            /* Check for another comma after skipping whitespace */
            if (line[i + 1] == ',') {
                return 1; /* Double commas found */
            }
        }
    }
    return 0; /* No double commas found */
}


/*******************************************************************************
 * Validates a data parameter.
 *
 * Parameters:
 * - param: The data parameter to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the data parameter is valid, 0 otherwise.
 ******************************************************************************/
int IsValidDataParam(char *param, int line_count){
    char *c; /* Pointer to navigate the parameter string */

    /* Check for spaces within the parameter, which are not allowed */
    if(strchr(param, ' ') || strchr(param, '\t')){
        fprintf(stderr, "Error at Line %d, Missing Comma\n", line_count);
        return 0;
    }

    /* Validate each character in the parameter */
    for(c = param; *c != '\0'; c++) {
        /* Allow an optional leading sign */
        if(*c == '-' || *c == '+') c++;
        /* Ensure the rest of the parameter is digits */
        if(!(isdigit(*c))){
            fprintf(stderr, "Error at Line %d, Extraneous text in data line\n", line_count);
            return 0;
        }
    }

    return 1; /* Parameter is valid */
}


/*******************************************************************************
 * Validates a string operand.
 *
 * Parameters:
 * - string: The string operand to validate.
 * - line_count: The current line number for error reporting.
 *
 * Returns:
 * - 1 if the string operand is valid, 0 otherwise.
 ******************************************************************************/
int IsValidString(char *string, int line_count){
    if(string == NULL){
        fprintf(stderr, "Error at Line %d, Missing Data parameters\n", line_count);
        return 0;
    }
    if(*string != '\"' || string[strlen(string)-1] != '\"'){
        fprintf(stderr, "Error at Line %d, Missing quotation mark%s\n", line_count, string);
        return 0;
    }

    if(string[strlen(string)] != '\0') {
        fprintf(stderr, "Error at Line %d, Extraneous text after end of string data\n", line_count);
        return 0;
    }

    return 1; /* String syntax is valid */
}
