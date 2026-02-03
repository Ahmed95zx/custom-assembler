#include "Assembler.h"

/**
 * SecondPass Function:
 * Encodes the .am file to  assembly instructions and data into machine code.
 * It updates Labels addresses in the Labels Table, encodes instructions into the Code array,
 * encodes data into the Data array, updates the Data Counter (DC), and flags entries in the
 * Labels Table if an entry line is encountered. Finally, it updates the final label addresses.
 *
 * Parameters:
 *   am_file - Pointer to the processed assembly file with macro expansion (.am file).
 *   table - Pointer to the LabelTable containing label definitions and addresses.
 *   Code - Array to hold the encoded instructions.
 *   Data - Array to hold the encoded data.
 *   PC - Program counters; PC[0] for instructions (IC) and PC[1] for data (DC).
 *   Error - Pointer to an integer flag indicating if an error has occurred.
 */
void SecondPass(FILE* am_file, LabelTable* table, signed short Code[], signed short Data[], int PC[], int* Error) {
    char line[MAX_LINE_LENGTH]; /* Buffer to store each line read from the file. */
    int line_count = 0; /* Line counter for error reporting and processing. */

   
    while (fgets(line, MAX_LINE_LENGTH, am_file)){
        line_count++; 
        /* Process each line to encode instructions and data. */
        ProcessLine(line, table, Code, Data, PC, line_count, Error); 
    }
    
    /* After processing all lines, update label addresses if no errors occurred. */
    if (!*Error) {
        reallocateLabels(table, Code, PC[0]);
    }
    /* Close the .am file. */
    fclose(am_file);
}





