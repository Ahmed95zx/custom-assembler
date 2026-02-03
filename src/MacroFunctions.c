#include "Assembler.h"

/*******************************************************************************
 * Initializes a MacroList structure.
 * Sets the head and tail pointers to NULL.
 *
 * Parameters:
 * - list: Pointer to the MacroList to initialize.
 ******************************************************************************/
void initMacroList(MacroList* list) {
    list->head = NULL;
    list->tail = NULL;
}


/*******************************************************************************
 * Initializes a LinesArray structure.
 * Sets the initial capacity and size, and allocates memory for the lines.
 *
 * Parameters:
 * - array: Pointer to the LinesArray to initialize.
 ******************************************************************************/
void initLinesArray(LinesArray* array) {
    array->capacity = 1;
    array->size = 0;
    array->lines = (char **)malloc(array->capacity * sizeof(char*));
    if (!array->lines) {
        fprintf(stderr, "Failed to allocate memory for lines array\n");
        exit(1);
    }
    array->lines[0] = NULL;

}


/*******************************************************************************
 * Creates a new Macro structure.
 *
 * Parameters:
 * - macro_name: The name of the macro.
 *
 * Returns:
 * - A pointer to the newly created Macro structure.
 ******************************************************************************/
Macro *createMacro(const char *macro_name) {
    int len = strlen(macro_name)+1;
    Macro *new_macro = malloc(sizeof(Macro));
    if (!new_macro) {
        fprintf(stderr, "Failed to allocate memory for new macro\n");
        exit(1);
    }
    new_macro->name = (char *)malloc(len);
    if (!new_macro->name) {
        fprintf(stderr, "Failed to allocate memory for new macro name\n");
        free(new_macro);
        exit(1);
    }
    strcpy(new_macro->name, macro_name);
    initLinesArray(&new_macro->linesArray);
    new_macro->next = NULL;
    return new_macro;
}


/*******************************************************************************
 * Extracts the macro name from a line of code.
 *
 * Parameters:
 * - line: The line of code to process.
 * - counter: Pointer to the line counter for error reporting.
 *
 * Returns:
 * - The extracted macro name, or NULL if not found.
 ******************************************************************************/
char* getMacroName(char* line, int* counter) {
    char* name;
    line += strlen(MCRSTRT);
    name = deleteSpaces(line);

    /* In case no macro name found*/
    if (!name || *name == '\0'){
        printf("Error at line %d: Missing macro name after macro definition\n", *counter);
        return NULL;
    }
    if(strchr(name, ' ') || strchr(name, '\t')){
        fprintf(stderr, "Error at line %d, Extra text after macro definition\n", *counter);
        return NULL;
    } 
    return name;
}


/*******************************************************************************
 * Inserts a new macro name into the macro list.
 *
 * Parameters:
 * - list: Pointer to the MacroList to modify.
 * - macro_name: The name of the macro to insert.
 ******************************************************************************/
void insertMacroName(MacroList* list, const char* macro_name) {

    Macro* new_macro = createMacro(macro_name);
    
    addMacroToList(list, new_macro);
}


/*******************************************************************************
 * Adds a macro to the macro list.
 *
 * Parameters:
 * - list: Pointer to the MacroList to modify.
 * - macro: Pointer to the Macro to add.
 ******************************************************************************/
void addMacroToList(MacroList* list, Macro* macro) {
    if (!list->head) { /*Empty list*/
        list->head = list->tail = macro;
    } else {
        list->tail->next = macro; /* Append to the end*/
        list->tail = macro;       /* Update the tail */
    }
}


/*******************************************************************************
 * Inserts a new line into the specified macro's lines array.
 *
 * Parameters:
 * - list: Pointer to the MacroList to modify.
 * - line: The line of code to insert.
 * - macro_name: The name of the macro to modify.
 ******************************************************************************/
void insertMacroLine(MacroList* list, char* line, const char* macro_name) {
    Macro *macro;
    for (macro = list->head; macro != NULL; macro = macro->next) {
        if (strcmp(macro->name, macro_name) == 0) {
            addLineToArray(&macro->linesArray, line);
            return;
        }
    }
}


/*******************************************************************************
 * Adds a line to the specified LinesArray.
 *
 * Parameters:
 * - array: Pointer to the LinesArray to modify.
 * - line: The line of code to add.
 ******************************************************************************/
void addLineToArray(LinesArray* array, char* line) {
    array->lines[array->size] = (char *)calloc(strlen(line)+1, sizeof(char));
    if(!array->lines[array->size]){
        fprintf(stderr,"Error, failed to Allocate memory for lines array");
        exit(1);
    }
    strcpy(array->lines[array->size], line);
    array->size++;
    
    if (array->size == array->capacity){
        resizeLinesArray(array);
    }

}


/*******************************************************************************
 * Finds and replaces a macro in the given line.
 *
 * Parameters:
 * - list: Pointer to the MacroList to search.
 * - line: The line of code to process.
 * - am_file: Pointer to the output file.
 *
 * Returns:
 * - 1 if a macro was found and replaced, 0 otherwise.
 ******************************************************************************/
int findAndReplaceMacro(MacroList* list, char* line, FILE* am_file) {
    int i, len;
    Macro* macro;
    char* name = (char *)malloc(strlen(line)+1);
    if(!name){
        fprintf(stderr, "Error, failed to allocate memory for macro name");
        exit(1);
    }
    strcpy(name, line);
    name = strtok(name, "\r\n");
    name = deleteSpaces(name);
    if(strchr(name, ' ') || strchr(name, '\t')){
        free(name);
        return 0;
    }
    macro = list->head;
    while(macro) {
        if (macro->name && (strcmp(name, macro->name) == 0)) {
            for (i = 0; i < macro->linesArray.size; i++) {
                if (macro->linesArray.lines[i]) {
                    len = strlen(macro->linesArray.lines[i]);
                    fwrite(macro->linesArray.lines[i], sizeof(char), len, am_file);
                }          
            }
            free(name);
            return 1;
        }
        macro = macro->next;
    }
    free(name);
    return 0;
}


/*******************************************************************************
 * Resizes the lines array to accommodate more lines.
 *
 * Parameters:
 * - array: Pointer to the LinesArray to resize.
 ******************************************************************************/
void resizeLinesArray(LinesArray* array) {
    char** new_array = NULL; 
    int newCapacity = array->capacity * 2, i;     /* Double the capacity*/
    
    /* Allocate new memory block for the array of pointers based on the new capacity*/
    new_array = realloc(array->lines, newCapacity * sizeof(char*));
    if (!new_array) {
        fprintf(stderr, "Failed to allocate memory for resizing lines array\n");
        exit(1);
    }
    
    array->lines = new_array;
    array->capacity = newCapacity;
    
    /* Initialize the newly allocated pointers to NULL*/
    for (i = array->size; i < newCapacity; i++) {
        array->lines[i] = NULL;
    }
}


/*******************************************************************************
 * Frees the memory allocated for the macro list.
 *
 * Parameters:
 * - list: Pointer to the MacroList to free.
 ******************************************************************************/
void freeMacroList(MacroList* list) {
    Macro* macro = list->head, *temp;
    while (macro != NULL) {
        temp = macro->next;
        freeMacro(macro);
        macro = temp;
    }

    
}


/*******************************************************************************
 * Frees the memory allocated for a macro.
 *
 * Parameters:
 * - macro: Pointer to the Macro to free.
 ******************************************************************************/
void freeMacro(Macro* macro) {
    if (macro) {
        free(macro->name);
        freeLinesArray(&macro->linesArray);
        free(macro);
        macro = NULL;
    }
}


/*******************************************************************************
 * Frees the memory allocated for the lines array.
 *
 * Parameters:
 * - array: Pointer to the LinesArray to free.
 ******************************************************************************/
void freeLinesArray(LinesArray* array) {
    int i;
    if (array) {
        for( i = 0; i < array->size; ++i) {
            if(array->lines[i]) free(array->lines[i]);
        }
        array->lines = NULL;
        array->size = 0;
        array->capacity = 0;
    }
    array = NULL;
}

