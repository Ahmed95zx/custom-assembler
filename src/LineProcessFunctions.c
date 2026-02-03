#include "Assembler.h"


/*******************************************************************************
 * Processes a line of assembly code.
 *
 * Parameters:
 * - source_line: The line of source code to process.
 * - table: Pointer to the LabelTable for label management.
 * - Code: Array to hold the encoded machine code.
 * - Data: Array to hold the data values.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 * - line_count: The current line number for error reporting.
 * - Error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void ProcessLine(char* source_line, LabelTable* table, signed short Code[], signed short Data[], int PC[], int line_count, int* Error) 
{
    char* line = NULL; /* Pointer to manipulate and process the source_line. */
    Label *tmp_label = NULL; /* Temporary pointer to hold label information. */
    char *label_name = NULL; /* Pointer to hold the extracted label name. */
    int is_label = 0; /* Flag to indicate if the current line defines a label. */
    
    line = source_line; 
    
    /* Check if the line defines a label. */
    if (IsLabelDefinition(line)){

        label_name = deleteSpaces(strtok(line, ":\r\n")); /* Extract the label name. */

        tmp_label = UpdateAddressAndGetLabel(label_name, table, PC); /* Update label address. */

        line = deleteSpaces(strtok(NULL, "\r\n")); /* Move to the next part of the line after the label. */

        is_label = 1; /* Set the flag indicating this line contains a label definition. */
    }
    
    /* Check if the line contains an assembly instruction and encode it. */
    if (IsInstructionLine(line)){
        EncodeInstruction(line, table, Code, PC, line_count, Error);
    } 
    /* Otherwise, process directives. */
    else {
        ProcessDirectives(line, table, Data, tmp_label, &is_label, PC, line_count, Error);
    }
}


/*******************************************************************************
 * Checks if a line is empty or a comment.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line is empty or a comment, 0 otherwise.
 ******************************************************************************/
int isEmptyOrComment(char* line){
    if(*line == '\0'|| *line == ';' || *line == '\n') return 1; 
    return 0;
}


/*******************************************************************************
 * Checks if a line starts with a specific word.
 *
 * Parameters:
 * - line: The line to check.
 * - word: The word to match.
 * - num: The length of the word.
 *
 * Returns:
 * - 1 if the line starts with the word, 0 otherwise.
 ******************************************************************************/
int startsWith(char* line, char* word, int num){
    if (strncmp (line, word, num) == 0){
        return 1;
    } else return 0;
}


/*******************************************************************************
 * Checks if a line defines a label.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line defines a label, 0 otherwise.
 ******************************************************************************/
int IsLabelDefinition(char *line){
    if(strchr(line, ':')) return 1;
    return 0;
}


/*******************************************************************************
 * Checks if a line contains an assembly instruction.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line contains an instruction, 0 otherwise.
 ******************************************************************************/
int IsInstructionLine(char *line){
    return *line != '.';
}


/*******************************************************************************
 * Checks if a line contains a data directive.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line contains a data directive, 0 otherwise.
 ******************************************************************************/
int IsDataDirective(char *line){
    if(startsWith(line, ".data", strlen(".data"))) return 1;
    return 0;
}


/*******************************************************************************
 * Checks if a line contains a matrix directive.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line contains a matrix directive, 0 otherwise.
 ******************************************************************************/
int IsMatrixDirective(char *line){
    if(startsWith(line, ".mat", strlen(".mat"))) return 1;
    return 0; 
}


/*******************************************************************************
 * Checks if a line contains a string directive.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line contains a string directive, 0 otherwise.
 ******************************************************************************/
int IsStringDirective(char *line){
    if(startsWith(line, ".string", strlen(".string"))) return 1; 
    return 0; 
}


/*******************************************************************************
 * Checks if a line contains an entry directive.
 *
 * Parameters:
 * - line: The line to check.
 * - is_label: Pointer to the label flag.
 * - line_count: The current line number.
 *
 * Returns:
 * - 1 if the line contains an entry directive, 0 otherwise.
 ******************************************************************************/
int IsEntryDirective(char *line, int *is_label, int line_count){
    if(startsWith(line, ".entry", strlen(".entry"))){
        if(*is_label){
            fprintf(stderr, "Warning at Line %d, unused Label defined\n", line_count);
            *is_label = 0; /* Reset the label flag as it's considered unused */
        }
        return 1; 
    }
    return 0;
}

/*******************************************************************************
 * Checks if a line contains an extern directive.
 *
 * Parameters:
 * - line: The line to check.
 *
 * Returns:
 * - 1 if the line contains an extern directive, 0 otherwise.
 ******************************************************************************/
int isExtern(char *line){
    if (startsWith(line, ".extern", strlen(".extern"))) return 1;
    return 0;
}

/*******************************************************************************
 * Deletes leading and trailing whitespace from a string.
 *
 * Parameters:
 * - str: The string to modify.
 *
 * Returns:
 * - A pointer to the modified string.
 ******************************************************************************/
char* deleteSpaces(char *str){
    int len;
    int i;
    if(str == NULL){
        return NULL;
    }
    while(isspace(*str) || *str == '\t') str++;
    if(str == NULL){
        return NULL;
    }
    len = strlen(str);
    for (i = len - 1; i >= 0; i--){
        if ((str[i]) != ' ' && str[i] != '\t') break;
    }
    str[i+1] = '\0';
    return str;
}