#include "Assembler.h"


/*******************************************************************************
 * Changes the file name extension to a new one.
 * Allocates memory for the new file name and returns it.
 *
 * Parameters:
 * - file_name: The original file name.
 * - extention: The new extension to be added.
 *
 * Returns:
 * - A pointer to the newly allocated string with the new file name, or NULL on failure.
 ******************************************************************************/
char* changeFileNameExtension(char* file_name,char* extension){
    int name_len, new_len;
    char* dot, *new_name;

    if (!file_name || !extension) return NULL;

    /* Find the last dot (.) in the filename */
    dot = strrchr(file_name, '.');
    if (dot) {
        name_len = dot - file_name; /* Length up to the dot (excluding it)*/
    } else {
        name_len = strlen(file_name); /* No extension*/
    }

    /*Allocate space: name + new extension + null terminator */
    new_len = name_len + strlen(extension) + 1;
    new_name = (char*)malloc(new_len);
    if (!new_name) {
        fprintf(stderr, "MemError, Failed to allocate Memory for file name Extension!\n");
        return NULL;
    }
 
    strncpy(new_name, file_name, name_len);
    new_name[name_len] = '\0'; 
    strcat(new_name, extension);

    return new_name;
}


/*******************************************************************************
 * Writes the object file with the given code, data, and counters.
 *
 * Parameters:
 * - Code: The code array to write.
 * - Data: The data array to write.
 * - counter: The counters for instruction and data.
 * - file_name: The original file name.
 ******************************************************************************/
void Write_object_file(signed short Code[], signed short Data[], int counter[], char *file_name) {
    static char encoding_table[] = {'a', 'b', 'c', 'd'};
    FILE *obj = NULL;
    int i, len, IC, DC;
    unsigned int addr = 100;
    char *filename = NULL;
    char line[MAX_LINE_LENGTH+1] = {'\0'};
    char address[SIZE_OF_ADDRESS+1]= {'\0'};
    char word[SIZE_OF_WORD+1]= {'\0'};
    char ICF[SIZE_OF_ADDRESS+1] = {'\0'};
    char DCF[SIZE_OF_ADDRESS+1] = {'\0'};
    IC = counter[0];
    DC = counter[1];

    /* Change file name extension */
    filename = changeFileNameExtension(file_name, OBJECT_EXT);
    obj = fopen(filename, "w+b");
    if (obj == NULL) {
        fprintf(stderr, "Error: Could not create file %s\n", filename);
        exit(1);
    }

    /* Write ICF and DCF */
    encodeCounter(encoding_table, IC, ICF);
    encodeCounter(encoding_table, DC, DCF);
    len = sprintf(line, "\t%s %s\n", ICF, DCF);
    fwrite(line, sizeof(char), len, obj);

    /* Write code */
    for (i = 0; i < IC; i++) {
        /* Get address and word representations */
        encodeBase4(encoding_table, addr, address, SIZE_OF_ADDRESS);
        encodeBase4(encoding_table, Code[i], word, SIZE_OF_WORD);
        /* Write address and word representations */
        len = sprintf(line, "%s\t%s\n", address, word);
        fwrite(line, sizeof(char), len, obj);

        /* Clear buffers */
        memset(line, '\0', MAX_LINE_LENGTH);
        memset(word, '\0', SIZE_OF_WORD);
        memset(address, '\0', SIZE_OF_ADDRESS);

        /* Increment address */
        addr++;
    }
    /* Write data */
    for (i = 0; i < DC; i++) {
        
        encodeBase4(encoding_table, addr, address, SIZE_OF_ADDRESS);
        encodeBase4(encoding_table, Data[i], word, SIZE_OF_WORD);

        len = sprintf(line, "%s\t%s\n", address, word);
        fwrite(line, sizeof(char), len, obj);


        memset(line, '\0', MAX_LINE_LENGTH);
        memset(word, '\0', SIZE_OF_WORD);
        memset(address, '\0', SIZE_OF_ADDRESS);

        addr++;
    }
    
    free(filename);
    fclose(obj);
}


/*******************************************************************************
 * Writes the extern and entry files based on the label table.
 *
 * Parameters:
 * - Labels: The label table containing all labels.
 * - file_name: The original file name.
 ******************************************************************************/
void Write_extern_entry_files(LabelTable* Labels, char* file_name) {
    static char encoding_table[] = {'a', 'b', 'c', 'd'}; /* Encoding table for base 4 */
    unsigned int i, len, pos; /* Position in the line buffer */
    FILE *ext = NULL, *ent = NULL; /* File pointers for extern and entry files */
    Reference* ref; /* Reference to the current label's references */
    Label* current_label; /* Current label being processed */
    char line[MAX_LINE_LENGTH] = {'\0'}; /* Line buffer for writing */
    char *ext_file = NULL; /* Extern file name */
    char *ent_file = NULL; /* Entry file name */
    char address[SIZE_OF_ADDRESS+1] = {'\0'}; /* Address buffer */

    /* Iterate over all labels */
    for (i = 0; i < Labels->table_size; i++) {
        current_label = Labels->Labels[i];
        while (current_label) {
            /* If extern, open file and write references */
            if (current_label->ext){
                if (!ext) {
                    ext_file = changeFileNameExtension(file_name, EXTERN_EXT);
                    
                    ext = fopen(ext_file, "w+b");
                    if (!ext) {
                        fprintf(stderr, "Error, could not create file %s\n", ext_file);
                        exit(1);
                    }
                }
                ref = current_label->ref;
                while (ref != NULL) {
                    pos = (ref->pos) + 100;
                    encodeBase4(encoding_table, pos, address, SIZE_OF_ADDRESS);
                    len = sprintf(line, "%s\t%s\n", current_label->name, address);
                    fwrite(line, sizeof(char), len, ext);
                    memset(line, '\0', MAX_LINE_LENGTH);
                    memset(address, '\0', SIZE_OF_ADDRESS);
                    ref = ref->next;
                }
            }

            /* If entry, open file and write entry */
            else if (current_label->ent) {
                if (!ent) {
                    ent_file = changeFileNameExtension(file_name, ENTRY_EXT);
                    ent = fopen(ent_file, "w+b");
                    if (!ent) {
                        fprintf(stderr, "Error, could not create file %s\n", ent_file);
                        exit(1);
                    }
                }
                pos = current_label->address;
                encodeBase4(encoding_table, pos, address, SIZE_OF_ADDRESS);
                len = sprintf(line, "%s\t%s\n", current_label->name, address);
                fwrite(line, sizeof(char), len, ent);
                memset(line, '\0', MAX_LINE_LENGTH);
                memset(address, '\0', SIZE_OF_ADDRESS);
            }
            current_label = current_label->next;
        }
    }

    /* Clean up */
    free(ext_file);
    free(ent_file);
    if (ext){ fclose(ext);}
    if (ent){ fclose(ent);}
}


/*******************************************************************************
 * Encodes a value in base 4 and stores it in the provided buffer.
 *
 * Parameters:
 * - encoding_table: The encoding table to use for conversion.
 * - x: The value to convert.
 * - buffer: The output buffer for the encoded value.
 * - len: The length of the output buffer.
 ******************************************************************************/
void encodeBase4(char encoding_table[], unsigned int x, char *buffer, int len){
    int i;
    signed short y;
    
    for (i = len-1; i >= 0; i--){
        y = x%4;
        buffer[i] = encoding_table[y];
        x >>= 2; 
    }
    
    while (i >= 0) {
        buffer[i] = '0';
        i--;
    }
}


/*******************************************************************************
 * Encodes the instruction counter value into the provided buffer.
 *
 * Parameters:
 * - encoding_table: The encoding table to use for conversion.
 * - x: The instruction counter value to encode.
 * - buf: The output buffer for the encoded value.
 ******************************************************************************/
void encodeCounter(const char encoding_table[], unsigned int x, char *buf) {
    char temp[SIZE_OF_ADDRESS]; 
    int len = 0, i;

    if (x == 0) {
        buf[0] = encoding_table[0]; 
        buf[1] = '\0';
        return;
    }

    while (x > 0) {
        temp[len++] = encoding_table[x % 4];
        x /= 4;
    }

    
    for (i = 0; i < len; i++) {
        buf[i] = temp[len - 1 - i];
    }
    buf[len] = '\0';
}

