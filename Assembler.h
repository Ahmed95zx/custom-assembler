#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h> 
#include <strings.h>
#include <math.h>
#include <ctype.h> 

#define MAX_LENGTH 256
#define MAX_LINE_LENGTH 81
#define MAX_LABEL 31
#define TABLE_SIZE 10
#define SIZE_OF_BITS 10
#define SIZE_OF_ADDRESS 4 
#define SIZE_OF_WORD 5
#define FACTOR 0.75
#define IMMEDIATE 0
#define LABEL 1
#define MATRIX 2
#define REGISTER 3
#define MCREND "mcroend"
#define MCRSTRT "mcro"
#define AFTER_MACRO_EXT ".am"
#define OBJECT_EXT ".ob"
#define ENTRY_EXT ".ent"
#define EXTERN_EXT ".ext"


/*macro structs:dynamic array*/
typedef struct LinesArray{
    char **lines; /*dynamic Array of line pointers */
    int size;  /*Number of lines currently stored*/
    int capacity;  /*Current capacity of the array*/
} LinesArray;
typedef struct Macro{
    char *name; /*name of macro*/
    LinesArray linesArray; /*macro Lines array*/
    struct Macro* next; /*next pointer*/
} Macro;

typedef struct MacroList{
    Macro *head; /* Pointer to the first macro in the list*/
    Macro *tail; /* Pointer to the last macro in the list*/
} MacroList;

/*Label structs: hash table*/
typedef struct reference {
    unsigned short pos;
    struct reference* next;
} Reference;
typedef struct Label{
    char* name;
    int ext;
    int ent;
    int mat;
    unsigned short address;
    Reference* ref;
    unsigned int dc;
    struct Label* next;
} Label;
typedef struct LabelTable {
    int table_size;
    int num_labels;
    Label** Labels; 
} LabelTable;

/*Assembler Functions Prototypes*/
FILE* PreAssembler(char* file_name);
LabelTable* FirstPass(FILE *amFile, int *errorFlag);
void SecondPass(FILE* am_file, LabelTable* table, signed short Code[], signed short Data[], int PC[], int* Error);


/* File Writing Functions */
char* changeFileNameExtension(char* file_name,char* extension);
void Write_object_file(signed short Code[], signed short Data[], int counter[], char *file_name);
void Write_extern_entry_files(LabelTable* Labels, char* file_name);
void get_word(char encoding_table[], signed short x, char* word);
void get_address(char encoding_table[], unsigned int x, char* address);
void encodeBase4(char encoding_table[], unsigned int x, char *buffer, int len);
void encodeCounter(const char encoding_table[], unsigned int x, char *buf);

/* Macro Functions Prototypes */
void initMacroList(MacroList* list);
void initLinesArray(LinesArray* array);
Macro *createMacro(const char *macro_name);
char* getMacroName(char* line, int* counter);
void insertMacroName(MacroList* list, const char* macro_name);
void addMacroToList(MacroList* list, Macro* macro);
void insertMacroLine(MacroList* list, char* line, const char* macro_name);
void addLineToArray(LinesArray* array, char* line);
int findAndReplaceMacro(MacroList* list, char* line, FILE* am_file);
void resizeLinesArray(LinesArray* array);
void freeMacroList(MacroList* list);
void freeMacro(Macro* macro);
void freeLinesArray(LinesArray* array);

/* Label Functions Prototypes */
void ProcessLabelDefinition(char *line, LabelTable *table, int lineCount, int *errorFlag);
void ProcessExternDefinition(char* line, LabelTable* table, int lineCount, int* errorFlag);
LabelTable* create_LabelTable(int size);
void CheckAndResizeTable(LabelTable* table);
void resizeLabelTable(LabelTable* table);
Label *createLabel(char *name, int ext, int mat);
unsigned int hash(char* str);
void addLabel(LabelTable* table, char* name, int ext , int mat, int *Label_error);
short int findLabel(LabelTable* table, char* name);
Label *UpdateAddressAndGetLabel(char *label_name, LabelTable *table, int PC[]);
void reallocateLabels(LabelTable* table, signed short Code[], int IC);
void freeLabelTable(LabelTable* table);


/* Instructions Encoding Functions Prototypes */
void EncodeInstruction(char *line, LabelTable *table, signed short Code[], int PC[], int line_count, int *Error);
int getNumOperand(int opcode);
void EncodeOperands(int numOprnd, char* line, LabelTable *table, signed short Code[], int opcode, int PC[], int line_count, int *Encoding_Error);
void encodeImmediate(char *operand, signed short Code[], int IC, int *word_count);
int encodeLabelOperand(char *operand, LabelTable *table, signed short Code[], int IC, int *word_count);
int encodeMatrixOperand(char *matrix, LabelTable* table, signed short Code[], int IC, int* word_count, int line_count);
int encodeRegisterOperand(char *operand, signed short Code[], int i, int numOprnd, int opcode, int IC, int *is_reg, int *word_count);
int getOpcode(char *inst);
void insertBin(signed short x, signed short Code[], int count);
void saveRef(Label *label, unsigned short address);
int getAddressingMode(char* operand, LabelTable* table);


/* Directive Processing Functions Prototypes */
void ProcessDirectives(char *line, LabelTable *table, signed short Data[], Label *tmp_label, int *is_label, int PC[], int line_count, int *Error);
void ProcessEntryLine(char* line, LabelTable* table, int line_count, int* Error);
void EncodeDataLine(char *line, signed short Data[], int PC[], int line_count, int *Error);
void EncodeStringLine(char *line, signed short Data[], int PC[], int line_count, int *Error);
int EncodeMatrixLine(char *line, signed short Data[], int PC[], int line_count, int *Error);


/* Validation Functions Prototypes */
int IsValidMacroName(char* macro_name, int* counter);
int validLabel(char *label_name, int line_count);
int IsValidInstSyntax(char *line, int numOprnd, int line_count);
int IsValidImmUse(int i, int opcode, int numOprnd , int line_count);
int isValidImmediate(char *operand, int line_count);
int isValidNum(char *num);
int isValidReg(char *operand, int line_count);
int IsValidRegUse(char *operand, int i, int numOprnd, int opcode, int line_count);
int isLegalBrackets(char *operand);
int ValidateAndParseMatrixOperand(char* operand, LabelTable *table, unsigned short regs[], char mat_name[], int line_count);
int IsValidDataSyntax(char *dataLine, int line_count);
int hasDoubleCommas(const char* line);
int IsValidDataParam(char *param, int line_count);
int IsValidString(char *string, int line_count);




/* Line Process Functions Prototypes */
void ProcessLine(char* source_line, LabelTable* table, signed short Code[], signed short Data[], int PC[], int line_count, int* Error);
int isEmptyOrComment(char* line);
int startsWith(char* line, char* word, int num);
int IsLabelDefinition(char *line);
int IsInstructionLine(char *line);
int IsDataDirective(char *line);
int IsMatrixDirective(char *line);
int IsStringDirective(char *line);
int IsEntryDirective(char *line, int *is_label, int line_count);
int isExtern(char *line);
char* deleteSpaces(char *str);