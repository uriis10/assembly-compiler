#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "main_header.h"

static int get_opcode_funct(COMMAND * com);
static char *encode_operand(char *operand);
static int encode_addressing_method(int addressingMethod);

/* initializes the command table */
void initialize_com_table()
{
	/* initializes the index "mov" (which represents the opcode and funct of the command) with the attributes of the "mov" command and so on for all of the commands */
	INIT(mov, IMMEDIATE | DIRECT | REG_DIRECT, DIRECT | REG_DIRECT);
	INIT(cmp, IMMEDIATE | DIRECT | REG_DIRECT, IMMEDIATE | DIRECT | REG_DIRECT);
	INIT(add, IMMEDIATE | DIRECT | REG_DIRECT, DIRECT | REG_DIRECT);
	INIT(sub, IMMEDIATE | DIRECT | REG_DIRECT, DIRECT | REG_DIRECT);
	INIT(lea, DIRECT, DIRECT | REG_DIRECT);
	INIT(clr, NONE, DIRECT | REG_DIRECT);
	INIT(not, NONE, DIRECT | REG_DIRECT);
	INIT(inc, NONE, DIRECT | REG_DIRECT);
	INIT(dec, NONE, DIRECT | REG_DIRECT);
	INIT(jmp, NONE, DIRECT | RELATIVE);
	INIT(bne, NONE, DIRECT | RELATIVE);
	INIT(jsr, NONE, DIRECT | RELATIVE);
	INIT(red, NONE, DIRECT | REG_DIRECT);
	INIT(prn, NONE, IMMEDIATE | DIRECT | REG_DIRECT);
	INIT(rts, NONE, NONE);
	INIT(stop, NONE, NONE);
}

/* check if com is a legal command name and if it is returns that command from the command table */
COMMAND *is_legal_com(char *com)
{
	int i;

	/* loops over the command table and looks for com */
	for (i = 0; i < MAX_OP_FUNCT; i++)
		if (strlen(com_table[i].name) && !strcmp(com, com_table[i].name))
			return com_table + i;

	return NULL;
}

/* check if string is a name of register */
int is_register(char *operand)
{
	/* the num of the register (bitween 0 to 7) */
	int numRegister;
	/* the length of the num */
	int numLength;

	remove_space(operand);

	/* the string is already with remove_space(), check if the first char is 'r' */
	if (*operand == REGISTER_ADDRESSING)
		if (strlen(operand) > 1) {	/* register must to be at least 2 */
			if (isspace(*(operand + 1)))	/* the number must be tie in */
				return FALSE;

			sscanf(operand + 1, "%d%n", &numRegister, &numLength);	/* take the number, and is length */
			if (numLength == strlen(operand + 1) && numRegister >= MIN_REGISTER && numRegister <= MAX_REGISTER)	/* checks that the number is the rest of the line, and if it's in the range (0-7) */
				return TRUE;
		}

	return FALSE;
}

/* check which kind is the operand, dont check syntax */
char get_operand_type(char *operand)
{
	remove_space(operand);

	/* if it start with IMMEDIATE_ADDRESSING (#) , it must be IMMEDIATE */
	if (*operand == IMMEDIATE_ADDRESSING)
		return IMMEDIATE;

	/* if it start with RELATIVE_ADDRESSING (%) , it must be RELATIVE */
	if (*operand == RELATIVE_ADDRESSING)
		return RELATIVE;

	/* check if it's a register */
	if (is_register(operand) == TRUE)
		return REG_DIRECT;

	/* checks if its a variable */
	if (is_legal_var(operand) == TRUE)
		return DIRECT;

	return EOF;
}

/* check if if string is a number */
static int is_num(char *str)
{
	/* could be + or - in the start */
	if (!(*str == MINUS || *str == PLUS || isdigit(*str)))
		return FALSE;

	/* check if all the rest is digits */
	for (str++; *str; str++)
		if (!isdigit(*str))
			return FALSE;

	return TRUE;
}

/* check if this operand is legal for the addressing Mode of the command, and than check syntex */
int is_legal_operand(char addressingModeCom, char *operand)
{
	/* get what is the operand type */
	char addressingModeOperand = get_operand_type(operand);

	/* checks that the addressing method is allowed in this operand */
	if (!(addressingModeCom & addressingModeOperand))
		return FALSE;

	/* checks that operand is a number */
	if (addressingModeOperand == (char)IMMEDIATE)
		return is_num(operand + 1);	/* return if the syntax is correct */

	/* if operand is relative it is checked that the label is legal */
	if (addressingModeOperand == (char)RELATIVE)
		return is_legal_var(operand + 1);	/* return if the syntax is correct */

	/* if operand is a label it is checked that its a legal label */
	if (addressingModeOperand == (char)DIRECT)
		return is_legal_var(operand);

	/* if operand is a register syntax is checked */
	if (addressingModeOperand == (char)REG_DIRECT)
		return is_register(operand);

	/* if this point is reached that meens EOF was returned by "get_operand_type()" so false should be returned */
	return FALSE;
}

/* encodes a command and its operands (if it has any)*/
int encode_com(COMMAND * com, char *operands)
{
	FILE *cmdFile;				/* holds the temporary file with the commands */
	int numOfOperands;			/* stores the number of operands the command takes */
	char *operand;				/* holds one of the operands if needed */
	char *encodedOperand;		/* holds the encoded operand */
	int status = TRUE;			/* program status */
	char *tmp = NULL;			/* for retval */

	/* save how many arguments this com get */
	numOfOperands = !!(com->dst_type) + !!(com->src_type);	/* calculates the amount of operators the command should recieve */

	remove_space(operands);

	/* if there should be operands its illegal to have them start with a comma */
	if (numOfOperands && operands && *operands == COMMA) {
		printf("Error: illegal comma on line ");
		return FALSE;
	}

	cmdFile = fopen(COM_TEMP_FILE, "a");	/* opens the temporary file for commands */

	/* encodes the command according to the number of operands it recieves */
	switch (numOfOperands) {
		case 0:
			if (!operands || !strlen(operands)) {	/* makes sure no extra text was entered after the command */
				/* encodes the command and adds 1 to "IC" */
				fprintf(cmdFile, "%03X %c\n", get_opcode_funct(com) << (ADDRESSING_METHOD_BITS + ADDRESSING_METHOD_BITS), ABSOLUTE_LINE);
				IC += numOfOperands + 1;
			} else {
				printf("Error: extra text after command on line ");
				status = FALSE;
			}
			break;

		case 1:
			/* checks that there is an operand */
			if (!operands || !strlen(operands)) {
				printf("Error: missing operand on line ");
				status = FALSE;
				break;
			}
			/* makes sure there isn't extra text after the command */
			if ((tmp = get_word(operands, SPACE)) != NULL) {
				printf("Error: extra text after command on line ");
				status = FALSE;
				free(tmp);
				break;
			}

			if ((status = is_legal_operand(com->dst_type, operands)) == FALSE) {	/* check if the operand is legal */
				printf("Error: illegal syntax or addressing method for operand on line ");
				break;
			}

			/* encodes the first word of the command into the command temporary file (the word with the opcode, funct and addressing methods) */
			fprintf(cmdFile, "%03X %c\n", (get_opcode_funct(com) << (ADDRESSING_METHOD_BITS + ADDRESSING_METHOD_BITS)) | encode_addressing_method(get_operand_type(operands)), ABSOLUTE_LINE);
			/* encodes the operand into the command temporary file */
			encodedOperand = encode_operand(operands);
			fprintf(cmdFile, "%s\n", encodedOperand);

			IC += numOfOperands + 1;	/* adds the number of words added to "IC" */

			free(encodedOperand);
			break;

		case 2:
			/* separates the two operands */
			operand = get_word(operands, COMMA);

			remove_space(operand);
			remove_space(operands);

			/* checks that there are actually two operands */
			if (operand == NULL || operands == NULL || !strlen(operand) || !strlen(operands)) {
				printf("Error: missing operand on line ");
				status = FALSE;
				if (operand)
					free(operand);
				break;
			}
			/* makes sure there arent two consecutive commas */
			if (*operands == COMMA) {
				printf("Error: two consecutive commas on line ");
				status = FALSE;
				free(operand);
				break;
			}
			/* makes sure there isn't extra text after the command */
			if ((tmp = get_word(operands, SPACE)) != NULL) {
				printf("Error: extra text after command on line ");
				status = FALSE;
				free(tmp);
				free(operand);
				break;
			}
			/* makes sure the operands are both legal */
			if ((status = is_legal_operand(com->src_type, operand)) == FALSE || (status = is_legal_operand(com->dst_type, operands)) == FALSE) {
				printf("Error: illegal syntax or addressing method for operand on line ");
				free(operand);
				break;
			}

			/* encodes the first word of the command into the command temporary file (the word with the opcode, funct and addressing methods) */
			fprintf(cmdFile, "%03X %c\n",
					(((get_opcode_funct(com) << ADDRESSING_METHOD_BITS) | encode_addressing_method(get_operand_type(operand))) << ADDRESSING_METHOD_BITS) |
					encode_addressing_method(get_operand_type(operands)), ABSOLUTE_LINE);

			/* encodes the first operand */
			encodedOperand = encode_operand(operand);
			fprintf(cmdFile, "%s\n", encodedOperand);
			free(encodedOperand);

			/* encodes the second operand */
			encodedOperand = encode_operand(operands);
			fprintf(cmdFile, "%s\n", encodedOperand);
			free(encodedOperand);

			IC += numOfOperands + 1;	/* adds the number of words added to "IC" */

			free(operand);
			break;
	}

	fclose(cmdFile);
	return status;				/* retuirns the program status (if there were errors or not) */
}

/* encodes an operand */
static char *encode_operand(char *operand)
{
	char *code;					/* stores the encoded operand */
	int temp;

	remove_space(operand);

	/* checks if the syntax of the operand is legal */
	if (is_legal_operand(IMMEDIATE | DIRECT | RELATIVE | REG_DIRECT, operand) == FALSE)
		return NULL;

	code = calloc(MAX_ENCODED_LINE + 1, sizeof(char));	/* uses "calloc" specifically to start "code" as an empty string */

	/* encodes the operand according to their addressing method */
	switch (get_operand_type(operand)) {
		case IMMEDIATE:
			sscanf(operand + 1, "%d", &temp);	/* reads the number to encode from the operand ("operand + 1" is used to skip over the '#') */
			temp &= TWELVE_MASK;	/* turnicates the number if its out of bounds for 12 bits */
			sprintf(code, "%03X %c", temp, ABSOLUTE_LINE);	/* encodes the number and adds the "absolute line" sign (the letter 'A') to "code" */
			return code;

			/* direct and relative addressing methods aren't handeled here. an empty string is returned */
		case DIRECT:
		case RELATIVE:
			return code;

		case REG_DIRECT:
			sprintf(code, "%03X %c", 1 << atoi(operand + 1), ABSOLUTE_LINE);	/* we know that the number of the register is between 0 and 7 and that its the last character in
																				   "operand" (it was checked in "is_legal_operand()" at the beggining of this function) so we can just
																				   convert it using "atoi()" without doing any checks */
			return code;

			/* the default case is never supposed to be reached, but a "default" case was added for safety */
		default:
			break;
	}

	return NULL;				/* this point shouldn't be reached. "return NULL" added only for safety */
}

/* gets a "COMMAND" objects opcode and funct */
static int get_opcode_funct(COMMAND * com)
{
	int i;

	/* loops over the command table and looks for a command with the same name as "com" that commands index is its opcode and funct (see how the command table is built) */
	for (i = 0; i < MAX_OP_FUNCT; i++) {
		if (com_table[i].name && !strcmp(com_table[i].name, com->name))
			return i;
	}

	return EOF;					/* EOF is returned if no match is found */
}

/* returns the encoded addresing method (bits 0-1 and 3-4 in the first encoded word of a command) */
static int encode_addressing_method(int addressingMethod)
{
	int code;					/* stores the encoded addresing method */

	/* makes sure a legal addressing method was entered */
	if (addressingMethod != IMMEDIATE && addressingMethod != DIRECT && addressingMethod != RELATIVE && addressingMethod != REG_DIRECT)
		return EOF;

	/* the value of an encoded addressing method is the number of times the addressing method constant was shifted in the defenition of the "ADDRESSING_METHODS" enum */
	for (code = 0; addressingMethod != 1 && addressingMethod; code++)	/* the check "!addressingMethod" is added only for safety */
		addressingMethod >>= 1;

	return code;
}
