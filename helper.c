#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "main_header.h"

/* removes the spaces at the beginning of str */
void remove_start_space(char *str)
{
	char *temp = str;

	if (!str)					/* makes sure that str isn't null */
		return;

	while (*temp && isspace(*temp))	/* loops until (temp == NULL) or until there are no more spaces at the beginning of temp */
		temp++;					/* removes the space at the beginning of temp */

	memmove(str, temp, strlen(temp) + 1);	/* copys temp to str */
}

/* removes the spaces at the end of str */
void remove_end_space(char *str)
{
	if (!str)					/* makes sure that str isn't null */
		return;

	while (*str && isspace(str[strlen(str) - 1]))	/* loops until (str == NULL) or until there are no more spaces at the end of str */
		str[strlen(str) - 1] = STRING_END;	/* the last character (a space) is changed to '\0' */
}

/* removes the spaces at the beginning and the end of str */
void remove_space(char *str)
{
	remove_start_space(str);
	remove_end_space(str);
}

/* returns the first sub-string in "str" that ends with "c" or NULL if "c" isn't in "str". "str" is set to be the part of "str" that is after the sub-string */
char *get_word(char *str, char c)
{
	char *temp;
	char *ptrStr;

	if (!str)					/* makes sure str isn't NULL */
		return NULL;

	ptrStr = str;

	/* if "c" is a white character the returned string should end with any white character */
	if (isspace(c))
		while (*ptrStr && !isspace(*ptrStr))	/* finds the first white character */
			ptrStr++;
	else
		ptrStr = strchr(str, c);	/* sets ptrStr to the first occurence of "c" */

	if (!ptrStr || !strlen(ptrStr))	/* makes sure there was an occurence of "c" */
		return NULL;

	*ptrStr++ = STRING_END;		/* cuts str into the two wanted strings */

	temp = malloc((strlen(str) + 1) * sizeof(char));
	strcpy(temp, str);			/* copys the first part of str into temp */

	memmove(str, ptrStr, strlen(ptrStr) + 1);	/* sets str to the seconed part of str */
	return temp;				/* returns the first part of str */
}
