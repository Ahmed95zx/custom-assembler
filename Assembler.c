#include "Assembler.h"

/*
 * Assembler Program
 * ----------------------
 * This program serves as the main entry point for a two-pass assembler.
 * It processes one or more assembly source files provided as command-line arguments,
 * performing macro preprocessing, label resolution, and code generation.
 * 
 * For each input file, the assembler:
 *   1. Preprocesses macros and expands them into a temporary file.
 *   2. Performs a first pass to build a symbol table and identify labels.
 *   3. Executes a second pass to encode assembly instructions and data into machine code.
 *   4. Handles errors gracefully, reporting issues and cleaning up resources as needed.
 *   5. Outputs the resulting object code, as well as external and entry label files.
 * 
 * The assembler is designed to handle multiple files in a single run, ensuring
 * that each file is processed independently and that all allocated resources are
 * properly released after processing each file.
 */
int main(int argc, char** argv){
    FILE *am_file = NULL; /* Pointer to the assembly file */
    char *file_name = NULL; /* Pointer to the file name */
    LabelTable* table = NULL; /* Pointer to a Labels table */
    signed short Code[MAX_LENGTH] = {0}; /* Array to store compiled code */
    signed short Data[MAX_LENGTH] = {0}; /* Array to store compiled data */
    int PC[2] = {0}; /* Program counters array s.t. PC[0] = IC , PC[1] = DC */
    int Error = 0 /*Error flag*/, i /*loop counter*/;
    

    /* Check if at least one file name is provided as argument, else return Error */
    if (argc < 2) {
        printf("Missing File Name!\n");
        return 1;
    }

    /* Loop through each input file */
    for (i = 1; i < argc; i++) {
        /* Allocate memory for storing file name */
        file_name = malloc(strlen(argv[i]) + 1);
        if (!file_name){
            fprintf(stderr, "Error: Failed to allocate memory for filename!\n");
            continue; 
        }
        strcpy(file_name, argv[i]);
        
        /* Pre-process the file for macros and return a new file pointer */
        am_file = PreAssembler(file_name);

        /*firstPass Process labels and return the Labels table for Second Pass */
        table = FirstPass(am_file, &Error);
        
        /* Encode assembly instructions into machine code */
        SecondPass(am_file, table, Code, Data, PC, &Error);

        /* Check for errors during compilation, if found clean all resources */
        if (Error) {
            fprintf(stderr, "Failed to Compile File %s\n", file_name);
            memset(Code, 0, MAX_LENGTH);
            memset(Data, 0, MAX_LENGTH);
            memset(PC, 0, 2);
            freeLabelTable(table);
            free(file_name);
            Error = 0;
            continue;
        }
        
        /* Write the compiled code and data to an object file */
        Write_object_file(Code, Data, PC, file_name);

        /* Write the external and entry labels to respective files */
        Write_extern_entry_files(table, file_name);
        
        /* Cleanup: reset arrays and free allocated memory */
        memset(Code, 0, MAX_LENGTH);
        memset(Data, 0, MAX_LENGTH);
        memset(PC, 0, 2);
        freeLabelTable(table);
        free(file_name);
        Error = 0;
        
    }
    return 0;
}
