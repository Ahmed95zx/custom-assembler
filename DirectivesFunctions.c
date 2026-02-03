#include "Assembler.h"

/*******************************************************************************
 * Processes directives in assembly source files.
 * Handles .data, .string, .matrix, and .entry directives,
 * encoding them into the Data array and updating the program counters.
 *
 * Parameters:
 * - line: The line containing the directive to be processed.
 * - table: Pointer to the LabelTable for label management.
 * - Data: Array to hold encoded data.
 * - tmp_label: Pointer to the current label being processed (if any).
 * - is_label: Pointer to a flag indicating if the line contains a label.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void ProcessDirectives(char *line, LabelTable *table, signed short Data[], Label *tmp_label, int *is_label, int PC[], int line_count, int *Error){
    int DC = PC[1]; /* Data counter (DC) for the .data, .string and .matrix directives */

    /* Process .data directive */
    if (IsDataDirective(line)) {
        /* If the directive is associated with a label, set the label's DC */
        if(*is_label) tmp_label->dc = DC;

        EncodeDataLine(line, Data, PC, line_count, Error);
    } 
    
    /* Process .string directive */
    else if (IsStringDirective(line)) {
        /* If the directive is associated with a label, set the label's DC */
        if(*is_label) tmp_label->dc = DC;
        /* Encode the .string line */
        EncodeStringLine(line, Data, PC, line_count, Error);
    }


    /* Process .matrix directive */
    else if (IsMatrixDirective(line)) {
        /* If the directive is associated with a label */
        if(!*is_label) {
            fprintf(stderr, "Error at Line %d: Missing label name for .matrix directive\n", line_count);
            *Error = 1;
            return;
        }
        tmp_label->dc = DC; /* Set the label's DC */
        /* Encode the .matrix line */
        if(!EncodeMatrixLine(line, Data, PC, line_count, Error)) {
            return;
        }
    }


    /* Process .entry directive */
    else if (IsEntryDirective(line, is_label, line_count)) {
        /* Process the .entry line to mark the label as an entry */
        ProcessEntryLine(line, table, line_count, Error);
    } 

    /* Handle unrecognized directives */
    else if (!isExtern(line)) {
        fprintf(stderr, "Error at Line %d: Unrecognized line format.\n", line_count);
        *Error = 1;
    }
}


/*******************************************************************************
 * Processes the .entry directive in assembly source files.
 * Marks the specified label as an entry point for external linking.
 *
 * Parameters:
 * - line: The line containing the .entry directive.
 * - table: Pointer to the LabelTable for label management.
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void ProcessEntryLine(char* line, LabelTable* table, int line_count, int* Error){
    int index; /* Index in the label table where the label is expected to be found */
    Label* current_label; /* Pointer to traverse labels in the table */
    char* entry_label; /* Pointer to store the label name specified in the .entry directive */

    /* Extract the label name from the line */
    line = strtok(line, " \r\n");
    entry_label = deleteSpaces(strtok(NULL, "\r\n"));

    /* Validate the presence of the label name */
    if(!entry_label || *entry_label == '\0'){
        fprintf(stderr, "Error at Line %d, Missing label name after entry definition:\n", line_count);
        *Error = 1; 
        return;
    }

    if(strchr(entry_label, ' ') || strchr(entry_label, '\t')){
        fprintf(stderr, "Error at Line %d, Extraneous text after entry label defined:\n", line_count);
        *Error = 1; 
        return;
    }

    /* Find the label in the table */
    if((index = findLabel(table, entry_label)) == -1){
        fprintf(stderr, "Error at Line %d, Undefined Label has been set as entry: %s\n", line_count, entry_label);
        *Error = 1; 
        return;
    }

    /* Traverse the linked list at the table index to find and mark the exact label as an entry */
    current_label = table->Labels[index];
    while(current_label){
        if(strcmp(current_label->name, entry_label) == 0){
            current_label->ent = 1;
            break;
        }
        current_label = current_label->next;
    }
}


/*******************************************************************************
 * Encodes a .data directive line in assembly source files.
 * Stores the immediate values or named constants in the Data array.
 *
 * Parameters:
 * - line: The line containing the .data directive.
 * - Data: Array to hold encoded data.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void EncodeDataLine(char *line, signed short Data[], int PC[], int line_count, int *Error){
    int imm; /* Variable to hold the immediate value or named constant value */
    int word_count = 0; /* Counter for the number of words added to the Data array */
    int DC = PC[1]; /* Data counter (DC) for the .data directive */
    char *param; /* Pointer to store each parameter in the .data directive after tokenizing */

    /* Extract the parameters from the line */
    line += strlen(".data");
    while(*line == ' ' || *line == '\t') line++;

    /* Validate the syntax of the data parameters */
    if(!IsValidDataSyntax(line, line_count)) {
        *Error = 1; 
        return;
    }
    
    /* Tokenize the parameters separated by commas */
    param = strtok(line, ",\r\n");
    while(param){
        param = deleteSpaces(param);

        /* Validate the parameter as a valid integer value */
        if(!IsValidDataParam(param, line_count)){
            *Error = 1; 
            return;
        }

        /* Convert the parameter to an integer */
        imm = atoi(param);
        
        /* Encode the integer value into the Data array at the current DC position */
        insertBin(imm, Data, (DC + word_count));
        word_count++; /* Increment the word counter */
        /* Get the next parameter */
        param = strtok(NULL, ",\r\n");
    }

    /* Update the DC value based on the number of words added */
    DC += word_count;
    PC[1] = DC; /* Update the program counter for data */
}

/*******************************************************************************
 * Encodes a .string directive line in assembly source files.
 * Stores the string characters in the Data array, including a null terminator.
 *
 * Parameters:
 * - line: The line containing the .string directive.
 * - Data: Array to hold encoded data.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void EncodeStringLine(char *line, signed short Data[], int PC[], int line_count, int *Error){
    int i = 0, word_count = 0, DC = PC[1]; /* Initialize counters and data counter (DC) */
    char *string = NULL; /* Pointer to the string to be encoded */

    /* Extract the string from the line */
    line = strtok(line, " \r\n");
    string = deleteSpaces(strtok(NULL, "\r\n"));

    /* Validate the string format */
    if(!IsValidString(string, line_count)){
        *Error = 1; /* Set error flag if the string is not valid */
        return;
    }

    string++; /* Move past the initial quotation mark */
    while(string[i] != '\"'){
        /* Encode each character of the string into the Data array */
        insertBin(string[i], Data, (DC + word_count));
        word_count++;  
        i++;
    }

    /* Encode the null terminator at the end of the string */
    Data[DC + word_count++] = '\0';
    DC += word_count; /* Update the data counter */
    PC[1] = DC; /* Update the program counter for data */
}

/*******************************************************************************
 * Encodes a .matrix directive line in assembly source files.
 * Stores the matrix elements in the Data array.
 *
 * Parameters:
 * - line: The line containing the .matrix directive.
 * - Data: Array to hold encoded data.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: Current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
int EncodeMatrixLine(char *line, signed short Data[], int PC[], int line_count, int *Error){
    int row = -1, col  = -1; /* Row and column indices for the matrix */
    unsigned short word_count = 0, L = 0, imm = 0; /* Word count and immediate value */
    int DC = PC[1]; /* Data counter (DC) for the .mat directive */
    char *index = NULL, *data = NULL, *param = NULL; /* Pointer to store each parameter in the matrix after tokenizing */

    /* Move past the .mat directive */
    line += strlen(".mat");
    while(*line == ' ' || *line == '\t') line++;

    /* Validate the syntax of the matrix parameters */
    if(!isLegalBrackets(line)){
        fprintf(stderr, "Error at Line %d, Illegal Brackets in .mat directive.\n", line_count);
        *Error = 1;
        return 0;
    }

    line++; /* Move past the initial bracket */

    /* Tokenize the line to get the row size */
    index = deleteSpaces(strtok(line, "]"));
    
    /* Validate each character in the parameter */
    if(!isValidNum(index)){
        fprintf(stderr, "Error at Line %d, Extraneous text in matrix definition line\n", line_count);
        *Error = 1;
        return 0;
    }

    row = atoi(index);
    if(row < 0){
        fprintf(stderr, "Error at Line %d, Invalid row count in matrix definition\n", line_count);
        *Error = 1; /* Set error flag if the row count is invalid */
        return 0;
    }

    
    
    index = deleteSpaces(strtok(NULL, "]"));
    if( *index != '['){
        fprintf(stderr, "Error at Line %d, Extraneous text in matrix definition line\n", line_count);
        *Error = 1;
        return 0;
    }
    index++;
    
    /* Validate each character in the parameter */
    if(!isValidNum(index)){
        fprintf(stderr, "Error at Line %d, Extraneous text in matrix definition line\n", line_count);
        *Error = 1;
        return 0;
    }

    col = atoi(index);
    if(col < 0){
        fprintf(stderr, "Error at Line %d, Invalid column count in matrix definition\n", line_count);
        *Error = 1; /* Set error flag if the column count is invalid */
        return 0;
    }

    word_count = row * col; /* Calculate the total number of words needed for the matrix */

    data = deleteSpaces(strtok(NULL, "\r\n"));
    if(data && *data != '\0'){
        
        if(!IsValidDataSyntax(data, line_count)){
            *Error = 1; /* Set error flag if the data syntax is invalid */
            return 0;
        }
        /* Tokenize the parameters separated by commas */
        param = strtok(data, ",\r\n");
        while(param){
            /* Trim any leading or trailing spaces */
            param = deleteSpaces(param);
            /* Validate the parameter as a valid integer value */
            if(!IsValidDataParam(param, line_count)){
                *Error = 1; /* Set error flag if the parameter is invalid */
                return 0;
            }
            /* Convert the parameter to an integer */
            imm = (unsigned short)atoi(param);
            /* Encode the integer value into the Data array at the current DC position */
            insertBin(imm, Data, (DC + L));
            L++; /* Increment the word counter */
            /* Get the next parameter */
            param = strtok(NULL, ",\r\n");
        }
    }

    /* Update the DC value based on the number of words added */
    DC += word_count;
    PC[1] = DC; /* Update the program counter for data */

    return 1;
}
