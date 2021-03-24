#include <stdio.h>																				/* included to allow prototypes with FILE * as parameter or as return value */

/* used a lot as return values */
#define FALSE 0
#define TRUE 1

#define MIN_LENGTH_LABEL 1																		/* the minimum length of label  */
#define TWELVE_MASK 0xFFF																		/* a mask used to "erase" every bit from bit 12 inclusive */
#define MAX_COM 4																				/* max length of a command (including '\0') */
#define MAX_LABEL_LENGTH 31																		/* max length of a variable name */
#define IC_START 100																			/* the starting value of IC */
#define MAX_LINE 85																				/* the max length of a line */
#define MAX_FILE_PREFIX_LEN 4																	/* the max prefix to a file name (e.g. ".as", ".tmp") including '.' excluding '\0' */
#define MAX_ENCODED_LINE 5																		/* the max number of characters in a line after it has been encoded (e.g. "FFF A" has exactly 5
																									characters) */

/* different types of characters used in the program */
#define SPACE ' '
#define END_LABEL ':'																			/* the symbol that supossed to has, in the end of label */
#define COMMA ','
#define QUOTE '\"'
#define COMMENT_CHAR ';'																		/* the symbol starts a comment line */
#define DOT '.'
#define NEW_LINE '\n'
#define PLUS '+'
#define MINUS '-'
#define STRING_END '\0'																			/* the symbol ends strings */

#define ADDRESSING_METHOD_BITS 2																/* the number of bits the type of addressing method takes up */
#define NUM_FUNCT_BITS 4																		/* the number of bits the funct feild takes up */
#define OP_FUNCT_BITS(opcode,funct)	((funct & 0xF) | ((opcode & 0xF) << NUM_FUNCT_BITS))		/* creates the funct and opcode feilds and combines them */
#define INIT(n,s,d) { COMMAND tmp = { #n, s, d }; com_table[n] = tmp; }							/* creates a new command and puts it in the command table */

#define DATA_TEMP_FILE "data.tmp"																/* temporary file for the data image */
#define COM_TEMP_FILE "command.tmp"																/* temporary file for the command image */
#define EXT_TEMP_FILE "external.tmp"															/* temporary file for information about the external variables */

#define FILE_PREFIX ".as"																		/* prefix for input files the program needs to open */
/* prefixs for output files created by the program */
#define OB_FILE_PREFIX ".ob"
#define ENT_FILE_PREFIX ".ent"
#define EXT_FILE_PREFIX ".ext"

/* the three types of words in the output ".ob" file */
#define ABSOLUTE_LINE 'A'
#define RELOCATABLE_LINE 'R'
#define EXTERNAL_LINE 'E'

/* the different types of instructions the program can encode */
#define DATA_WORD "data"
#define STRING_WORD "string"
#define EXTERN_WORD "extern"
#define ENTRY_WORD "entry"

#define IMMEDIATE_ADDRESSING '#'																/* the symbol that signal the immediate addressing method */
#define RELATIVE_ADDRESSING '%'																	/* the symbol that signal the relative addressing method */
#define REGISTER_ADDRESSING 'r'																	/* the symbol that signals the begining of what could be a register name */

#define MIN_REGISTER 0																			/* the minimum of the register prefix range */
#define MAX_REGISTER 7																			/* the maximum of the register prefix name*/

/* a single command (the command table is an array of these commands) */
typedef struct {
	char name[MAX_COM];																			/* the command name */
	char src_type;																				/* the allowed source addressing methods this command can recieve */
	char dst_type;																				/* the allowed destination addressing methods this command can recieve */
} COMMAND;

/* creates constants with the opcode and funct of every command (and a constant with the maximum opcode and funct) */
typedef enum {
	mov = OP_FUNCT_BITS(0,0),
	cmp = OP_FUNCT_BITS(1,0),
	add = OP_FUNCT_BITS(2,10),
	sub = OP_FUNCT_BITS(2,11),
	lea = OP_FUNCT_BITS(4,0),
	clr = OP_FUNCT_BITS(5,10),
	not = OP_FUNCT_BITS(5,11),
	inc = OP_FUNCT_BITS(5,12),
	dec = OP_FUNCT_BITS(5,13),
	jmp = OP_FUNCT_BITS(9,10),
	bne = OP_FUNCT_BITS(9,11),
	jsr = OP_FUNCT_BITS(9,12),
	red = OP_FUNCT_BITS(12,0),
	prn = OP_FUNCT_BITS(13,0),
	rts = OP_FUNCT_BITS(14,0),
	stop = OP_FUNCT_BITS(15,0),
	MAX_OP_FUNCT
} COM_OP_FUNCT;

enum ADDRESSING_METHODS { NONE, IMMEDIATE = 1, DIRECT = 1<<1, RELATIVE = 1<<2, REG_DIRECT = 1<<3 };	/* constants for the types of addressing methods (NONE is used to set the source/destination
																										addressing methods to "doesn't receive source/destination  operand" */
enum LINE_TYPE {COM_L = 2, DATA_L, STRING_L, ENTRY_L, EXTERN_L};									/* constants for the types of lines */
enum ATTRIBUTES {CODE, DATA, EXTERN, CODE_ENTRY, DATA_ENTRY};										/* constants for variable attributes */

/* a single variable (the symbol table is a linked list of these) */
typedef struct var {
	char *name;																						/* name of the variable */
	int defLine;																					/* the line the variable was defined on */
	int attribute;																					/* the "attribute" is one of the constants in the "ATTRIBUTES" enum */
	struct var *next;																				/* the next variable in the symbol table linked list */
} VARIABLE;

extern COMMAND com_table[];																			/* the command table */
extern VARIABLE *head;																				/* head of the symbol table linked list */
extern int DC;																						/* counts the number of data words there are */
extern int IC;																						/* counts the number of command words there are */

/* function prototypes */
int save_var(char *, int, int);
void remove_space(char *);
void remove_start_space(char *);
void remove_end_space(char *);
void initialize_com_table();
int is_legal_var(char *);
COMMAND *is_legal_com(char *);
char *get_word(char *, char);
char *find_var(char *);
int encode_data(char *);
int encode_string(char *);
void update_data_vars(void);
int update_entry_vars(char *, int);
int first_reading(FILE *);
int second_reading(FILE *);
void create_output(char *);
VARIABLE *get_var(char *);
void free_symbol_table(void);
char get_operand_type(char *);
int is_legal_operand(char, char *);
int encode_com(COMMAND * ,char *);
int is_register(char *);
int encode_var_operand(VARIABLE *, int);
