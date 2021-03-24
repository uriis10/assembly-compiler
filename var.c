#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "main_header.h"

static VARIABLE *define_new_variable(char *labelName, int defLine, int attribute);

/* get a name of a label, is define line, and his attribute.
   and than save this label in the variable table */
int save_var(char *labelName, int defLine, int attribute)
{
	/* pointer to VARIABLE, help with the for loop */
	VARIABLE *pointer;

	/* check if is a legel var name */
	if (is_legal_var(labelName) == FALSE)
		return FALSE;

	/* if the variable table is empty, put the new variable in the first position */
	if (head == NULL) {
		head = define_new_variable(labelName, defLine, attribute);
		return TRUE;
	}

	/* else... put the new variable in the end of the table */
	for (pointer = head; pointer->next != NULL; pointer = pointer->next)
		;
	pointer->next = define_new_variable(labelName, defLine, attribute);

	return TRUE;
}

/* create a new label */
static VARIABLE *define_new_variable(char *labelName, int defLine, int attribute)
{
	VARIABLE *newVariable = malloc(sizeof(VARIABLE));

	/* take all the parametrs, and put it's in the newVariable */

	if (labelName) {
		newVariable->name = malloc((strlen(labelName) + 1) * sizeof(char));
		strcpy(newVariable->name, labelName);
	}
	newVariable->defLine = defLine;
	newVariable->attribute = attribute;
	newVariable->next = NULL;

	return newVariable;
}

/* return the label in the start of the line, if it exists. else return null. */
char *find_var(char *str)
{
	char *saveStr;
	char *labelName;

	/* remove the space from the start */
	remove_start_space(str);

	/* check if the length of the line is makes sense, else return NULL */
	if (!str || strlen(str) < MIN_LENGTH_LABEL + 1)
		return NULL;

	/* save the original string, for case that will be a problem with the label */
	saveStr = malloc((strlen(str) + 1) * sizeof(char));
	strcpy(saveStr, str);

	/* take the first word feom the line */
	labelName = get_word(str, SPACE);

	/* check labelName is a label (end with ':'), if it is, so return labelName */
	if (labelName && labelName[strlen(labelName) - 1] == END_LABEL) {
		labelName[strlen(labelName) - 1] = STRING_END;
		free(saveStr);
		return labelName;
	}

	/* labelName is a label isn't, so return str to is start position, and return NULL */
	strcpy(str, saveStr);
	free(saveStr);
	if (labelName)
		free(labelName);
	return NULL;
}

/* check if var is a legal variable name */
int is_legal_var(char *varName)
{
	int i;

	if (!varName || !isalpha(*varName))	/* makes sure var isn't NULL and that its first character is a letter */
		return FALSE;

	if (strlen(varName) > MAX_LABEL_LENGTH)
		return FALSE;

	/* loops over var and checks that every character is a digit or letter */
	for (i = 1; varName[i]; i++)
		if (!isalnum(varName[i]))
			return FALSE;

	if (is_register(varName) == TRUE)
		return FALSE;

	/* check if the variable is a command which is illegal */
	if (is_legal_com(varName))
		return FALSE;

	/* chaeck if the variable is "data", "string", "entry" or "extern" which is illegal */
	if (!strcmp(varName, DATA_WORD) || !strcmp(varName, STRING_WORD) || !strcmp(varName, ENTRY_WORD) || !strcmp(varName, EXTERN_WORD))
		return FALSE;

	return TRUE;
}

/* in the end of the first transition, plus IC to all "data" labels */
void update_data_vars()
{
	VARIABLE *pointer = head;
	for (pointer = head; pointer != NULL; pointer = pointer->next)
		if (pointer->attribute == DATA)
			pointer->defLine += IC;
}

/* in the second transition, change the label to be entry too */
int update_entry_vars(char *labelName, int lineNumber)
{
	VARIABLE *label;

	remove_space(labelName);

	/* looks for the label in the symbpl table, prints error if the label isn't found */
	if (!(label = get_var(labelName))) {
		printf("Error: the variable %s is declared to be \"entry\" but it is never defined on line %d\n", labelName, lineNumber);
		return FALSE;
	}

	/* check if its "code" label */
	if (label->attribute == CODE) {
		/* change it to be a "entry too" */
		label->attribute = CODE_ENTRY;
		return TRUE;
	}
	/* check if its ".data" label */
	if (label->attribute == DATA) {
		/* change it to be a "entry too" */
		label->attribute = DATA_ENTRY;
		return TRUE;
	}

	/* if this point is reached that meens the variable was declared to be both "entry" and "extern" which is illegal */
	printf("Error: the variable %s is declared to be both \"entry\" and \"extern\" on line %d\n", labelName, lineNumber);
	return FALSE;
}

/* looks for a variable in the symbol table by name */
VARIABLE *get_var(char *varName)
{
	VARIABLE *pointer;
	/* loops until a variable with the name varName is found or the end of the table is reached */
	for (pointer = head; pointer != NULL; pointer = pointer->next) {
		if (!strcmp(varName, pointer->name))
			return pointer;
	}

	/* returns NULL if the variable wasn't found */
	return NULL;
}

/* frees the symbol table */
void free_symbol_table()
{
	VARIABLE *var;
	VARIABLE *temp;

	/* loops over the symbol table and frees all of the names and "VARIABLE" objects */
	for (var = head; var != NULL; var = temp) {
		temp = var->next;		/* temp is "moved" one variable forward to allow "free(var)" but also allow var to keep move "down" the table */

		free(var->name);
		free(var);
	}
	head = NULL;				/* a defense against segmentation faults that might be caused by the fact that freeing a pointer doesn't set it to NULL (since
								   the symbol table is global, the freed-but-not-NULL "VARIABLE"'s can cause problems with functions that try to access the
								   name of a "VARIABLE" if "VARIABLE" isn't NULL like "VARIABLE *get_var(char *)" */
}

/* encodes a variable that is an operand to a command */
int encode_var_operand(VARIABLE * var, int operandType)
{
	FILE *comTmpFile;			/* command temporary file */
	FILE *tmp;					/* a temporary file */
	char c1;					/* a character from the command temporary file */
	char c2;					/* one character after "c1" */
	long location;				/* a location in a file */
	long i;						/* needs to be "long" to allow it to get to values similar to "location" */
	int lineNumber = 0;			/* the offset of the current line from "IC_START" */

	comTmpFile = fopen(COM_TEMP_FILE, "r+");	/* opens the file for reading and writing (it's known that the file exists so theres no need to check that) */

	c1 = getc(comTmpFile);		/* reads the first character from the file */
	/* looks for the first two '\n' characters in a row, between them the encoding of the variable (the spot was left empty in the first reading) */
	for (c2 = getc(comTmpFile); (c1 != NEW_LINE || c2 != NEW_LINE) && c1 != EOF; c2 = getc(comTmpFile))	/* the check "c1 != EOF" is for safety. it makes sure the loop wont be endless */
		c1 = c2;

	/* for safety only, program shouldn't reach this point. makes sure the end of the file wasn't reached */
	if (c1 == EOF)
		return EOF;

	location = ftell(comTmpFile) - 1;	/* saves the current location in the command temporary file */
	rewind(comTmpFile);			/* the stream is currently after the second '\n' so it's moved one character backwards */

	tmp = tmpfile();			/* creates a temporary file */

	/* copys "comTmpFile" up to "location" to the temporary file */
	for (i = 0; i < location; i++) {
		c1 = getc(comTmpFile);
		putc(c1, tmp);
		/* adds to the line number if a new line has been reached */
		if (c1 == NEW_LINE)
			lineNumber++;
	}

	/* encodes the variable to the temporary file. the type of line (A, R, E) is determined by the variables attribute and the operand type. if the operand is in relative addressing then the
	   relative addressing formula: {vriable defenition line} - ({current line} + 1). the "+1" is added when the line number counted because it counts the first word of the command as a line even 
	   though technicaly the line number should be one line backwards (its known that relative addressing can only be used in command with one operand) */
	fprintf(tmp, "%03X %c", (var->defLine - ((operandType == RELATIVE) ? lineNumber + IC_START : 0)) & TWELVE_MASK,
			(var->attribute == EXTERN) ? EXTERNAL_LINE : ((operandType == RELATIVE) ? ABSOLUTE_LINE : RELOCATABLE_LINE));

	/* copys the rest of the command temporary file to the temporary file */
	while ((c1 = getc(comTmpFile)) != EOF)
		putc(c1, tmp);

	/* rewinds the file position of "tmp" and "comTmpFile" to the beggining */
	rewind(tmp);
	rewind(comTmpFile);

	/* copys "tmp" into "comTmpFile" */
	while ((c1 = getc(tmp)) != EOF)
		putc(c1, comTmpFile);

	fclose(tmp);				/* this also erases "tmp" */
	fclose(comTmpFile);
	return lineNumber;
}
