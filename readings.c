#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main_header.h"

static char *get_line(FILE * sourceFile);
static int encode_line(char *line, int lineNumber);
static int find_line_type(char *str);
static int second_reading_com_operand(char *varOperand);

/* executes the first reading an assembler does on a ".as" file */
int first_reading(FILE * sourceFile)
{
	char *line;
	int lineNumber = 0;
	int status = TRUE;			/* FALSE if there was an error in one of the lines, TRUE otherwise */

	while ((line = get_line(sourceFile))) {
		lineNumber++;

		/* if the line is empty or a comment it is skipped */
		if (!strlen(line) || *line == COMMENT_CHAR) {
			free(line);
			continue;
		}

		status = (encode_line(line, lineNumber) == TRUE) ? status : FALSE;	/* if there was ever an error status will be error, true otherwise */
		free(line);
	}

	update_data_vars();			/* adds IC to the "defLine" of every variable with a "data" attribute */

	return status;
}

/* executes the first reading an assembler does on a ".as" file */
int second_reading(FILE * sourceFile)
{
	char *line;					/* the current line */
	int lineNumber = 0;
	int status = TRUE;			/* the status of the program */
	int lineStatus;				/* the status of the current line */
	char *temp;
	char *tempWord = NULL;		/* used to make sure allocated is freed */

	/* loops over all the lines in the source file and encodes operands that are variables and updates the "entry" status of variables that are operand of a ".entry" instruction */
	while ((line = get_line(sourceFile))) {
		lineNumber++;			/* increments the line number */
		lineStatus = TRUE;		/* initializes the line status to "TRUE" */

		/* if the line is empty or a comment it is skipped */
		if (!strlen(line) || *line == COMMENT_CHAR) {
			free(line);
			continue;
		}

		/* skips variable definition (if theres one on this line) */
		tempWord = find_var(line);
		if (tempWord)
			free(tempWord);
		remove_space(line);

		tempWord = get_word(line, SPACE);
		/* chooses the correct action according to the type of line */
		switch (find_line_type(tempWord)) {
			case ENTRY_L:
				lineStatus = update_entry_vars(line, lineNumber);	/* updates the "entry" status of the variable */
				break;
			case COM_L:
				remove_space(line);

				/* if there are no operands the line is skipped */
				if (!strlen(line))
					break;

				/* checks if there are two operands */
				if ((temp = get_word(line, COMMA))) {
					if ((lineStatus = second_reading_com_operand(temp)) == FALSE) {	/* encodes the operand if its a veriable. on error a message is printed to the standard output */
						printf("%d\n", lineNumber);	/* the error message was printed in the function "var_second_reading()", here only the line number is printed */
						free(temp);
						break;
					}
					free(temp);
				}
				if ((lineStatus = second_reading_com_operand(line)) == FALSE)	/* encodes the operand if its a veriable. on error a message is printed to the standard output */
					printf("%d\n", lineNumber);	/* the error message was printed in the function "var_second_reading()", here only the line number is printed */

				break;
		}

		status = (lineStatus == TRUE) ? status : lineStatus;	/* if there was an error on a previos line "status" stays "FALSE" otherwise it's set to the line status */
		free(tempWord);
		free(line);
	}

	return status;				/* return the program status */
}

/* handles an operand of a command during the second reading */
static int second_reading_com_operand(char *varOperand)
{
	FILE *extTmpFile;			/* the temporary file for external variables */
	VARIABLE *var;				/* will hold the variable the "varOperand" might be */
	int operandType;			/* stores the operand type (direct/relative) */
	int operandWordNumber;

	remove_space(varOperand);
	/* checks if the operand is a variable */
	switch ((operandType = get_operand_type(varOperand))) {
		case RELATIVE:			/* if the operand is a variable with a "relative" addressing method the '%' should be skipped and it should be checked 
								   that the operand isnt an external variable (if its a variable). no "break" has been put at the end because except for 
								   these couple extra line "relative" and "direct" addressing methods are handled the same */
			varOperand++;
			if ((var = get_var(varOperand)) && var->attribute == EXTERN) {
				printf("Error: the variable %s is used in relative addressing but it is external on line ", varOperand);
				return FALSE;
			}
		case DIRECT:
			if (!(var = get_var(varOperand))) {	/* finds the variable in the symbol table. prints error if it doesn't exist */
				printf("Error: the variable %s is never defined but it is used on line ", varOperand);
				return FALSE;
			}

			operandWordNumber = encode_var_operand(var, operandType);	/* encodes the variable (inserts it's "defLine" to the correct place in the command
																		   image */
			/* if the variable is external this use of it is added to the temporary file of external variables */
			if (var->attribute == EXTERN) {
				extTmpFile = fopen(EXT_TEMP_FILE, "a");
				fprintf(extTmpFile, "%s %04d\n", var->name, operandWordNumber + IC_START);
				fclose(extTmpFile);
			}

			return TRUE;		/* no errors, TRUE should be returned */

		default:				/* if the operand isn't a variable nothing is done with it in the second reading but that isn't an error */
			return TRUE;
	}

	return FALSE;				/* this point shouldn't be reached. the "return" is only for safety */
}

/* encodes a line (a string) */
static int encode_line(char *line, int lineNumber)
{
	char *var;
	int status = TRUE;
	char *typeWord;				/* the word that determines if its a data/command/extern/... line (i. e. ".data", "mov" and so on) */
	int lineType;

	var = find_var(line);		/* gets the variable declared on the current line (if one is declared on the line) */
	remove_space(line);

	/* gets the key word in this line. if there isn't one an error is printed */
	if (!strlen(line)) {
		printf("Error: missing command or instruction on line %d\n", lineNumber);
		free(var);
		return FALSE;
	}
	/* if theres only one word left in "line" then that has to be the type word */
	if (!(typeWord = get_word(line, SPACE))) {
		typeWord = line;
		line = NULL;
	}

	lineType = find_line_type(typeWord);	/* gets the line type */

	/* prints specific error for a second definition of a variable (on ".entry" and ".extern" lines its only a warning) */
	if (var && get_var(var)) {
		if (lineType == ENTRY_L || lineType == EXTERN_L)
			printf("Warning: illegal variable defenition, vriable defined a second time on line %d\n", lineNumber);
		else
			printf("Error: illegal variable defenition, vriable defined a second time on line %d\n", lineNumber);
	}

	/* does a spicific action according to the type of the keyword (the type of line) */
	switch (lineType) {
		case DATA_L:
			if (var && (status = save_var(var, DC, DATA)) == FALSE)	/* tries to save the variable to the symbol table (repeats in most of the cases) */
				printf("Error: illegal variable defenition on line %d\n", lineNumber);
			else if ((status = encode_data(line)) == FALSE)	/* encodes the list of numbers that come after ".data" */
				printf("%d\n", lineNumber);	/* the error message was printed in the function "encode_data()", here only the line number is printed */
			break;
		case STRING_L:
			if (var && (status = save_var(var, DC, DATA)) == FALSE)
				printf("Error: illegal variable definition on line %d\n", lineNumber);
			else if ((status = encode_string(line)) == FALSE)	/* encodes the list of numbers that come after ".data" */
				printf("%d\n", lineNumber);	/* the error message was printed in the function "encode_string()", here only the line number is printed */
			break;
		case EXTERN_L:
			if (var && is_legal_var(var) == FALSE)
				printf("Warning: illegal variable defininition on line %d\n", lineNumber);

			if ((status = save_var(line, 0, EXTERN)) == FALSE)	/* saves the variable thats an operand of ".extern" */
				printf("Error: illegal operand to \".extern\" instruction on line %d\n", lineNumber);
			break;
		case ENTRY_L:			/* ".entry" lines are skipped (they're handeled in the second reading */
			if (var && is_legal_var(var) == FALSE)
				printf("Warning: illegal variable definition on line %d\n", lineNumber);
			break;
		case COM_L:
			if (var && (status = save_var(var, IC, CODE)) == FALSE)	/* tries to save the variable to the symbol table */
				printf("Error: illegal variable definition on line %d\n", lineNumber);
			else if ((status = encode_com(is_legal_com(typeWord), line)) == FALSE)	/* encodes the command and its operands */
				printf("%d\n", lineNumber);	/* the error message was printed in the function "encode_com()", here only the line number is printed */
			break;
		default:				/* if the program enters this case that meens that an unknown key word has been written */
			if (var && is_legal_var(var) == FALSE)
				printf("Warning: illegal variable decloration on line %d\n", lineNumber);

			printf("Error: unknown command or instruction \"%s\" on line %d\n", typeWord, lineNumber);
			status = FALSE;
			break;
	}

	if (line)					/* if (line == NULL) then a free(line) (which is the same as free(typeWord)) will be called in a different place */
		free(typeWord);
	if (var)
		free(var);

	return status;
}

/* gets a non-comment and non-empty line from a file */
static char *get_line(FILE * sourceFile)
{
	char *line = malloc(MAX_LINE * sizeof(char));

	/* gets a line from the source file returns NULL if EOF is encountered */
	if (!fgets(line, MAX_LINE, sourceFile)) {
		free(line);
		return NULL;
	}

	remove_space(line);			/* removes all the white characters from the beginning and end of the line */

	return line;
}

/* get a word, and return what is the kind of the line the word represents */
static int find_line_type(char *keyWord)
{
	if (!keyWord)
		return FALSE;

	remove_space(keyWord);

	if (*keyWord != DOT)
		/* check if it's a command line */
		if (is_legal_com(keyWord))
			return COM_L;
		else
			return FALSE;
	else
		keyWord++;

	/* check if it's data */
	if (!strcmp(keyWord, DATA_WORD))
		return DATA_L;

	/* check if it's string */
	if (!strcmp(keyWord, STRING_WORD))
		return STRING_L;

	/* check if it's entry */
	if (!strcmp(keyWord, ENTRY_WORD))
		return ENTRY_L;

	/* check if it's extern */
	if (!strcmp(keyWord, EXTERN_WORD))
		return EXTERN_L;

	return FALSE;
}
