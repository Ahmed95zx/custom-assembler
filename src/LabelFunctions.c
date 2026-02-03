#include "Assembler.h"

/*******************************************************************************
 * Processes a label definition in an assembly source file.
 * Adds the label to the label table with its address and type (external or matrix).
 *
 * Parameters:
 * - line: The line containing the label definition.
 * - table: Pointer to the LabelTable for label management.
 * - lineCount: Current line number for error reporting.
 * - errorFlag: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void ProcessLabelDefinition(char *line, LabelTable *table, int lineCount, int *errorFlag){

    char *label_name = NULL; /* Pointer to store the name of the label. */
    char *line_rest = NULL;
    int mat = 0;

    label_name = deleteSpaces(strtok(line, ":\r\n"));
    if (!validLabel(label_name, lineCount)) {
        *errorFlag = 1; 
        return;
    }

    if (findLabel(table, label_name) != -1) {
        fprintf(stderr, "Error at line %d, Duplicate Label definition %s\n", lineCount, label_name);
        *errorFlag = 1;
        return;
    }
    line_rest = deleteSpaces(strtok(NULL, "\r\n"));
    mat = IsMatrixDirective(line_rest);

    addLabel(table, label_name, 0, mat, errorFlag);
}


/*******************************************************************************
 * Processes an extern label definition in an assembly source file.
 * Adds the label to the label table with its address and type (external).
 *
 * Parameters:
 * - line: The line containing the extern label definition.
 * - table: Pointer to the LabelTable for label management.
 * - lineCount: Current line number for error reporting.
 * - errorFlag: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void ProcessExternDefinition(char* line, LabelTable* table, int lineCount, int* errorFlag) {
    char *label_name = NULL; /* Pointer to store the name of the external label. */

    
    line += strlen(".extern");
    label_name = deleteSpaces(line);
    if (!label_name || isEmptyOrComment(label_name)) {
        fprintf(stderr, "Error at Line %d: Missing extern label name\n", lineCount);
        *errorFlag = 1; /* Set error flag. */
        return;
    }

    if(strchr(label_name, ' ') || strchr(label_name, '\t')) {
        fprintf(stderr, "Error at Line %d: Extra text after extern label definition\n", lineCount);
        *errorFlag = 1; /* Set error flag. */
        return;
    }

    label_name = deleteSpaces(label_name);

    
    if (!validLabel(label_name, lineCount)) {
        *errorFlag = 1; /* Set error flag. */
        return;
    }

    if (findLabel(table, label_name) != -1) {
        fprintf(stderr, "Error at line %d, Duplicate extern label definition %s\n", lineCount, label_name);
        *errorFlag = 1;
        return;
    }
    addLabel(table, label_name, 1, 0, errorFlag);/* Add the label as an external label. */
}


/*******************************************************************************
 * Creates a new label table.
 *
 * Parameters:
 * - size: The initial size of the label table.
 *
 * Returns:
 * - A pointer to the newly created LabelTable.
 ******************************************************************************/
LabelTable* create_LabelTable(int size){
    int i;
    LabelTable* table = (LabelTable *)malloc(sizeof(LabelTable)); /* Allocate memory for label table */
    if(table == NULL){
        fprintf(stderr, "Error, Failed to allocate memory for the Labels table\n");
        exit(1);
    } 
    table->table_size = size; /* Set initial table size */
    table->Labels = (Label **)malloc(size * sizeof(Label *)); /* Allocate memory for label pointers */
    if(table->Labels == NULL){
        fprintf(stderr, "Error, Failed to allocate memory for the Labels\n");
        exit(1);
    } 
    table->num_labels = 0; /* Initialize number of labels to 0 */
    for(i = 0; i < size; i++){ /* Initialize label pointers to NULL */
        table->Labels[i] = NULL;
    }
    return table; /* Return the pointer to the newly created label table */
}


/*******************************************************************************
 * Checks and resizes the label table if necessary.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to check.
 ******************************************************************************/
void CheckAndResizeTable(LabelTable* table) {
    if (table->num_labels >= FACTOR * table->table_size) {
        resizeLabelTable(table);
    }
}


/*******************************************************************************
 * Resizes the label table to accommodate more labels.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to resize.
 ******************************************************************************/
void resizeLabelTable(LabelTable* table) {
    Label *current_label = NULL, *next_label = NULL;
    int old_size = table->table_size;
    int new_size = old_size * 2;
    int i = 0, index = 0;

    /*Allocate new array of pointers to Label*/
    Label** new_labels = calloc(new_size, sizeof(Label*));
    if (!new_labels) {
        fprintf(stderr, "Error: Failed to allocate memory for the new Labels table\n");
        exit(1);
    }
    

    /*Rehash labels into the new table*/
    for (i = 0; i < old_size; i++) {
        current_label = table->Labels[i];
        while (current_label != NULL) {
            next_label = current_label->next; /*Store next label before modifying the current one*/
            /*Calculate new index based on the new size, if your label structure or addLabel involves more complex logic, adjust accordingly*/ 
            index = hash(current_label->name) % new_size;
            current_label->next = new_labels[index]; /* Insert at beginning of the list in the new table*/
            new_labels[index] = current_label;
            current_label = next_label; /*Move to the next label in the old table*/
        }
    }

    /*Free old table and update the table structure*/
    free(table->Labels);
    table->Labels = new_labels;
    table->table_size = new_size;
}


/*******************************************************************************
 * Creates a new label.
 *
 * Parameters:
 * - name: The name of the label.
 * - ext: The external flag for the label.
 * - mat: The matrix flag for the label.
 *
 * Returns:
 * - A pointer to the newly created Label.
 ******************************************************************************/
Label *createLabel(char *name, int ext, int mat){
    Label *new_label = (Label *)malloc(sizeof(Label));
    new_label->name = NULL; 
    new_label->name = (char *)malloc(MAX_LABEL);
    if(new_label->name == NULL){
        fprintf(stderr, "Failed to allocate memory for new Label\n");
        exit(1);
    }
    strcpy(new_label->name, name);
    new_label->address = 0;
    new_label->ext = ext;
    new_label->ent = 0;
    new_label->mat = mat;
    new_label->ref = NULL;
    new_label->dc = 0;
    new_label->next = NULL;
    
    return new_label;
}


/*******************************************************************************
 * Hashes a string to produce an index for the label table.
 *
 * Parameters:
 * - str: The string to hash.
 *
 * Returns:
 * - The hashed index.
 ******************************************************************************/
unsigned int hash(char* str){
    unsigned int hash = 0;
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        hash = (hash << 3) ^ str[i]; /*Bitwise XOR to provide a better distribution*/
    }
    return hash;
}


/*******************************************************************************
 * Adds a label to the label table.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to modify.
 * - name: The name of the label.
 * - ext: The external flag for the label.
 * - mat: The matrix flag for the label.
 * - Label_error: Pointer to an integer flag indicating if an error has occurred.
 ******************************************************************************/
void addLabel(LabelTable* table, char* name, int ext , int mat, int *Label_error){
    unsigned int index;
    Label *current_label = NULL; 
    Label *new_label;
    
    new_label = createLabel(name, ext, mat);
    index = hash(name) % (table->table_size);
    current_label = table->Labels[index];
    if(current_label == NULL) table->num_labels++;
    new_label->next = current_label;
    table->Labels[index] = new_label;
    
    CheckAndResizeTable(table);
    
    
    
}


/*******************************************************************************
 * Finds a label in the label table.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to search.
 * - name: The name of the label to find.
 *
 * Returns:
 * - The index of the label if found, or -1 if not found.
 ******************************************************************************/
short int findLabel(LabelTable* table, char* name){
    unsigned int index;
    Label *current_label;
    index = hash(name) % table->table_size;
    current_label = table->Labels[index];
    while (current_label != NULL){
        if (strcmp(current_label->name, name) == 0) {
            return index;
        }
        current_label = current_label->next;
    }
    return -1; 
}


/*******************************************************************************
 * Updates the address of a label in the label table and returns a pointer to the label.
 *
 * Parameters:
 * - label_name: The name of the label to update.
 * - table: Pointer to the LabelTable to modify.
 * - PC : Program counters array (PC[0] for IC, PC[1] for DC).
 *
 * Returns:
 * - A pointer to the updated Label, or NULL if not found.
 ******************************************************************************/
Label *UpdateAddressAndGetLabel(char *label_name, LabelTable *table, int PC[]){
    Label *current_label; /* Pointer to traverse through labels in the table */
    int index; /* Index where the label is expected to be found based on hashing or similar mechanism */

    /* Validate input parameters */
    if (!label_name || !table) return NULL;

    /* Find the index of the label in the table */
    index = findLabel(table, label_name);

    /* Validate the index */
    if (index < 0 || index >= table->table_size) return NULL;

    /* Traverse the linked list at the found index to search for the label */
    for (current_label = table->Labels[index]; current_label; current_label = current_label->next) {
        /* Compare the current label's name with the target label name */
        if (strcmp(current_label->name, label_name) == 0) {
            /* If found, update the label's address */
            current_label->address = PC[0] + PC[1] + 100;
            /* Return the pointer to the updated label */
            return current_label;
        }
    }

    /* Return NULL if the label is not found */
    return NULL;
}


/*******************************************************************************
 * Reallocates the label table to accommodate more labels.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to resize.
 ******************************************************************************/
void reallocateLabels(LabelTable* table, signed short Code[], int IC) {
    Reference* ref;
    Label* current_label;
    int i;
    /* Iterate through all labels in the table */
    for (i = 0; i < table->table_size; i++) {
        current_label = table->Labels[i];
        while(current_label){
            /* Update label addresses and references */
            ref = current_label->ref;
            while (ref != NULL){
                Code[ref->pos] |= (current_label->address << 2); /* Update the code with the label's address */
                ref = ref->next;
            }
            current_label = current_label->next;
        }
    }
}


/*******************************************************************************
 * Frees the memory allocated for the label table.
 *
 * Parameters:
 * - table: Pointer to the LabelTable to free.
 ******************************************************************************/
void freeLabelTable(LabelTable* table){
    int i;
    Label *current_label, *next_label;
    Reference *current_ref, *next_ref;
    
    for(i = 0; i < table->table_size; i++){
        current_label = table->Labels[i];
        while (current_label != NULL) {
            next_label = current_label->next;
            free(current_label->name);
            current_ref = current_label->ref;
            while(current_ref){
                next_ref = current_ref->next;
                free(current_ref);
                current_ref = next_ref;
            }
            free(current_label);
            current_label = next_label;
        }
    }
    if(table->Labels) free(table->Labels);
    table->Labels = NULL;
    free(table);
}



