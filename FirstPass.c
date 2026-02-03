#include "Assembler.h"

/*******************************************************************************
 * First Pass of the Assembler
 * -----------------------------
 * This function performs the first pass of the assembler, processing labels and
 * external definitions within an assembly file.
 *
 * Parameters:
 *   amFile - Pointer to the assembly file after macro processing.
 *   errorFlag - Pointer to an integer flag to indicate errors.
 *
 * Returns:
 *   A pointer to the label table if successful, NULL otherwise.
 ******************************************************************************/
LabelTable* FirstPass(FILE *amFile, int *errorFlag) {
    LabelTable *table = NULL; /* Pointer to the label table. */
    char line[MAX_LINE_LENGTH]; /* Buffer to hold each line read from the file. */
    int lineCount = 0; /* Counter to track the current line number. */
    int len = 0; /* Length of the current line. */

    table = create_LabelTable(TABLE_SIZE);

    /* Read each line of the file until the end is reached. */
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        lineCount++; 
        
        if (isExtern(line) || IsLabelDefinition(line)){
            len = strlen(line)-1;
            while(line[len] == '\r' || line[len] == '\n'){
                line[len] = '\0';
                len--;
            }
        if (isExtern(line))
            ProcessExternDefinition(line, table, lineCount, errorFlag);
        else
            ProcessLabelDefinition(line, table, lineCount, errorFlag);
        }
    }

    rewind(amFile);
    return table;
}
