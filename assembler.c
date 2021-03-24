#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_header.h"

COMMAND com_table[MAX_OP_FUNCT] = { 0 };	/* a table that stores all the information about all of the commands */

VARIABLE *head;					/* pointer to the first element in the symbol (table) linked list */
int DC;							/* counts the number of data lines */
int IC = IC_START;				/* counts the number of instruction lines */

static void encode_file(char *sourceName);
static void create_tmp_files();
static void remove_tmp_files();

int main(int argc, char **argv)
{
	if (argc <= 1) {
		printf("No files entered to process\n");
		return 0;
	}

	initialize_com_table();		/* initializes a table that stores all of the information about the commands in the assembly language */

	/* loops over all of the files passed through the command line and encodes them */
	for (argc--; argc > 0; argc--) {
		IC = IC_START;			/* makes sure IC's value doesn't stay the same from a previous file */
		DC = 0;

		create_tmp_files();		/* creates (or erases) temporary files used in the program */
		encode_file(argv[argc]);	/* encodes the current file */
	}
	remove_tmp_files();			/* removes the temporary files */

	return 0;
}

/* encodes a file */
static void encode_file(char *argv)
{
	FILE *sourceFile;
	char *sourceName;

	/* adds ".as" to the name of the source file */
	sourceName = malloc((strlen(argv) + strlen(FILE_PREFIX) + 1) * sizeof(char));
	strcpy(sourceName, argv);
	strcat(sourceName, FILE_PREFIX);

	printf("\n--------------- %s -------------\n", sourceName);

	/* tries to open the source file. an error is printed if the action fails */
	if (!(sourceFile = fopen(sourceName, "r"))) {
		printf("Error: The file \"%s\" doesn't exist or program doesn't have access to it\n", sourceName);
		free(sourceName);
		return;
	}

	/* processes the file the first time (stops if there are errors) */
	if (first_reading(sourceFile) == FALSE) {
		fclose(sourceFile);
		free(sourceName);
		free_symbol_table();
		return;
	}

	fclose(sourceFile);
	/* tries to open the source file. an error is printed if the action fails */
	if (!(sourceFile = fopen(sourceName, "r"))) {
		printf("There was a problem opening \"%s\", its encoding has been stopped\n", sourceName);
		free(sourceName);
		free_symbol_table();
		return;
	}

	/* processes the file the second time (stops if there are errors) */
	if (second_reading(sourceFile) == FALSE) {
		fclose(sourceFile);
		free(sourceName);
		free_symbol_table();
		return;
	}

	create_output(argv);		/* creates output for the source file */

	fclose(sourceFile);
	free_symbol_table();

	printf("%s has been encoded successfully\n", sourceName);
	free(sourceName);
}

/* creates (or erases) temporary files used in the program */
static void create_tmp_files()
{
	FILE *tempFile;

	/* "w" is used when opening to erase the files if they already exist */
	tempFile = fopen(DATA_TEMP_FILE, "w");
	fclose(tempFile);
	tempFile = fopen(COM_TEMP_FILE, "w");
	fclose(tempFile);
	tempFile = fopen(EXT_TEMP_FILE, "w");
	fclose(tempFile);
}

/* removes the temporary files used in the program */
static void remove_tmp_files()
{
	remove(DATA_TEMP_FILE);		/* removes the data image temporary file */
	remove(COM_TEMP_FILE);		/* removes the command image temporary file */
	remove(EXT_TEMP_FILE);		/* removes the external variable temporary file */
}
