#include "Assembler.h"

/*******************************************************************************
 *MacroProcess function
 * Processes assembly source files to handle macros.
 * Reads input file line by line, expanding macros where defined and used,
 * Output a new file with a ".am" extension where macros have been expanded.
 * 
 * Parameters:
 * - file_name: The name of the source assembly file to be processed.
 * 
 * Returns:
 * - A FILE pointer to the newly created file after macro processing.
 ******************************************************************************/
FILE* PreAssembler(char* file_name) {
    
    FILE* Source_file = NULL, *am_file = NULL; /* Pointers to the source and assembly files */
    char* am_file_name = NULL; /* Pointer to the assembly file name */

    char source_line[MAX_LINE_LENGTH] = {0}; /* Buffer for reading source lines */
    char* line = NULL; /* Pointer to the current line being processed */

    MacroList macroList = {0}; /* Initialize macro list */
    char macro_name[MAX_LINE_LENGTH] = {0}; /* Buffer for macro names */
    int inside_macro = 0, counter = 0; /* Flag detecting inside macro and line counter */

    char* tmp_buffer; /* Temp buffer for check usage */

    /* Open the source file for reading */
    Source_file = fopen(file_name, "r");
    if (!Source_file) {
        fprintf(stderr, "Error opening file: %s\n", file_name);
        return NULL;
    }

    /* Create the .am file for writing the processed output,
     Return allocated memory contain the new file name */
    am_file_name = changeFileNameExtension(file_name, AFTER_MACRO_EXT);
    if(!am_file_name){
        fclose(Source_file);
        return NULL;
    }
    am_file = fopen(am_file_name, "w+b");
    if (!am_file){
        fprintf(stderr, "Error, Failed to create file: %s\n", am_file_name);
        /* Cleanup and return NULL on failure */
        free(am_file_name);
        fclose(Source_file);
        return NULL;
    }
    /* Free the allocated filename string as it's no longer needed */
    free(am_file_name);

    /* Initialize the macro list structure */
    initMacroList(&macroList);

    /* Read the source file line by line */
    while (fgets(source_line, MAX_LINE_LENGTH, Source_file)) {
        counter++; /* Line counter for error messages */
        /* Remove leading and trailing spaces from the read line */
        line = deleteSpaces(source_line);
        
        /* Skip empty lines or comments */
        if (isEmptyOrComment(line)) continue;
        
        /* Handling macro end marker */
        if (startsWith(line, MCREND, strlen(MCREND))) {

            /*Check for extra text after macro end*/
            line += strlen(MCREND);
            tmp_buffer = deleteSpaces(line);
            if(isEmptyOrComment(tmp_buffer)  == 0){
                fprintf(stderr, "Error at line %d, Extra Text after macro end.",counter);
                return NULL;
            }

            inside_macro = 0; /* Not inside a macro anymore */
            continue;
        }
        
        /* Handling macro start marker */
        if (startsWith(line, MCRSTRT, strlen(MCRSTRT))) {

            /* Extract and validate macro name */
            strcpy(macro_name, getMacroName(line, &counter));
            if(!IsValidMacroName(macro_name, &counter)) continue;

            /* Insert the macro name into the macro list */
            insertMacroName(&macroList, macro_name);
            inside_macro = 1; /* Now inside a macro definition */
            continue;
        }
        
        /* If inside a macro, insert the line into the macro's content */
        if (inside_macro) 
            insertMacroLine(&macroList, line, macro_name);
        else {
            /* If not inside a macro, try to find and replace any macros used in the line */
            if(!findAndReplaceMacro(&macroList, line, am_file)){
                /* If no macro replacement occurred, write the original line to the .am file */
                fwrite(line, sizeof(char), strlen(line), am_file);
            }
        }
    }

    /* Cleanup: Free allocated resources and close files */
    freeMacroList(&macroList);
    fclose(Source_file);

    /* Reset and return the ".am" file pointer to the beginning for further processing */
    rewind(am_file);
    return am_file;
}

