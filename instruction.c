#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main_header.h"

/* encode a list of numbers into the data image */
int encode_data(char *toEncode)
{
	FILE *dataFile;
	int num = 0;
	int scanCheck = 0;
	char *element = NULL;
	int charCount = 0;

	remove_space(toEncode);

	/* makes sure toEncode isn't NULL and holds at least one character and that it doesn't end or start with a comma */
	if (!toEncode || !strlen(toEncode)) {
		printf("Error: no numbers entered after \".data\" instruction on line ");
		return FALSE;
	}

	dataFile = fopen(DATA_TEMP_FILE, "a");

	/* loops over every element of the data list and, if it a number, encodes it */
	do {
		if (element && element != toEncode)	/* free previour iteration allocation */
			free(element);

		if (!(element = get_word(toEncode, COMMA)))	/* gets the next element */
			element = toEncode;

		remove_space(element);

		/* prints error if there are two commas in a row or an extra comma before/after the list of numbers */
		if (!strlen(element)) {
			printf("Error: illegal comma on line ");
			fclose(dataFile);
			if (element != toEncode)	/* free allocation */
				free(element);
			return FALSE;
		}

		scanCheck = sscanf(element, "%d%n", &num, &charCount);	/* reads a number out of the element */

		if (scanCheck < 1 || strlen(element) != charCount) {	/* makes sure the element doesnt have any non-number character in it */
			printf("Error: illegal operand to \".data\" instruction (should be a single whole number) on line ");
			fclose(dataFile);
			if (element != toEncode)	/* free allocation */
				free(element);
			return FALSE;
		}

		fprintf(dataFile, "%03X %c\n", num & TWELVE_MASK, ABSOLUTE_LINE);	/* encodes the element */

		DC++;
	} while (strcmp(toEncode, element));	/* checks if the last element was just encoded */

	fclose(dataFile);
	return TRUE;
}

/* encode a string into the data image */
int encode_string(char *toEncode)
{
	FILE *dataFile;

	remove_space(toEncode);

	/* makes sure toEncode isn't NULL and holds at least two '"' characters and that it starts and ends with '"' */
	if (!toEncode || strlen(toEncode) <= 1 || !(*toEncode == QUOTE && toEncode[strlen(toEncode) - 1] == QUOTE)) {
		printf("Error: illegal operand to \".string\" instruction (should be a string) on line ");
		return FALSE;
	}

	/* "erases" the quotation marks */
	toEncode++;
	toEncode[strlen(toEncode) - 1] = STRING_END;

	/* opens the temporary file with the data image */
	dataFile = fopen(DATA_TEMP_FILE, "a");

	/* loops over every character in toEncode and encodes it */
	do {
		fprintf(dataFile, "%03X %c\n", (unsigned)(*toEncode), ABSOLUTE_LINE);
		DC++;
	} while (strlen(toEncode++));

	fclose(dataFile);
	return TRUE;
}
