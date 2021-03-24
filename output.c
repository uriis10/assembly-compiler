#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_header.h"

static int copy_line(FILE * from, FILE * to);
static void create_ob_file(char *fileName);
static void create_ent_file(char *fileName);
static void create_ext_file(char *fileName);

/* creates the output for a ".as" file */
void create_output(char *sourceName)
{
	char *tempName;

	tempName = malloc((strlen(sourceName) + MAX_FILE_PREFIX_LEN + 1) * sizeof(char));

	strcat(strcpy(tempName, sourceName), OB_FILE_PREFIX);	/* creates the name of the ".ob" file */
	create_ob_file(tempName);	/* creates the ".ob" file itself */

	strcat(strcpy(tempName, sourceName), EXT_FILE_PREFIX);	/* creates the name of the ".ext" file */
	create_ext_file(tempName);	/* creates the ".ext" file itself */

	strcat(strcpy(tempName, sourceName), ENT_FILE_PREFIX);	/* creates the name of the ".ent" file */
	create_ent_file(tempName);	/* creates the ".ent" file itself */

	free(tempName);

}

/* creates the ".ob" file */
static void create_ob_file(char *fileName)
{
	FILE *obFile;				/* the ".ob" file */
	FILE *tempFile;				/* will hold the programs temporary files */
	int i;

	obFile = fopen(fileName, "w");

	fprintf(obFile, "\t%d %d\n", IC - IC_START, DC);	/* prints the size of the the command image and the data image */

	tempFile = fopen(COM_TEMP_FILE, "r");	/* opens the tmp file with the command image */
	/* copys the command image to the ".ob" file */
	for (i = IC_START; i < IC; i++) {
		fprintf(obFile, "%04d ", i);	/* prints the memory address (starts at "IC_START" which is 100) */
		copy_line(tempFile, obFile);	/* copys a line from the command temporary file to the ".ob" file */
	}
	fclose(tempFile);

	tempFile = fopen(DATA_TEMP_FILE, "r");	/* opens the tmp file with the data image */
	/* copys the data image to the ".ob" file */
	for (; i < IC + DC; i++) {
		fprintf(obFile, "%04d ", i);	/* prints the memory address */
		copy_line(tempFile, obFile);	/* copys a line from the data temporary file to the ".ob" file */
	}
	fclose(tempFile);

	fclose(obFile);
}

/* creates the ".ent" file */
static void create_ent_file(char *fileName)
{
	FILE *entFile = NULL;		/* the ".ent" file */
	VARIABLE *pointer;			/* used for looping over the symbol table */

	/* loops over the symbol table and looks for variables with a "entry" attribute */
	for (pointer = head; pointer != NULL; pointer = pointer->next) {
		if (pointer->attribute == DATA_ENTRY || pointer->attribute == CODE_ENTRY) {
			/* if the ".ent" file hasn't been created/opened yet it should be created/opened now */
			if (!entFile)
				entFile = fopen(fileName, "w");

			fprintf(entFile, "%s %04d\n", pointer->name, pointer->defLine);	/* prints the name of the variable and the line it was defined on to the ".ent" file */
		}
	}

	/* closes the ".ent" file only if it was opened */
	if (entFile)
		fclose(entFile);
}

/* creates the ".ext" file */
static void create_ext_file(char *fileName)
{
	FILE *extFile;
	FILE *tmpFile;
	char c;

	if (!(tmpFile = fopen(EXT_TEMP_FILE, "r")))
		return;

	c = fgetc(tmpFile);
	if (c == EOF) {
		fclose(tmpFile);
		return;
	} else {
		ungetc(c, tmpFile);
	}

	extFile = fopen(fileName, "w");
	while (copy_line(tmpFile, extFile) == TRUE)
		;

	fclose(tmpFile);
	fclose(extFile);
}

/* copys a line from "from" to "to" */
static int copy_line(FILE * from, FILE * to)
{
	char *line = malloc(MAX_LINE * sizeof(char));
	if (!fgets(line, MAX_LINE, from)) {	/* gets the line to copy from "from" */
		free(line);
		return FALSE;
	}

	if (fputs(line, to) == EOF) {	/* prints the copied line to "to" */
		free(line);
		return FALSE;
	}

	free(line);
	return TRUE;
}
