/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.05.29.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <unistd.h>
#include <string.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

/*
 * Check whether a string is made only by numbers or not
 * Input:   name, the string
 * Output:  1,    if the string is only made by numbers
 * 			0,    else
 */
int isOnlyMadeByNumbers(const char *name)
{
	regex_t regExp;
	char *pattern = "[0-9]";
	int status;

    if (regcomp(&regExp, pattern, REG_EXTENDED) != 0)
    	errorAndDie("regcomp");

    status = regexec(&regExp, name, regExp.re_nsub, NULL, 0);
    regfree(&regExp);

    return !status;
}

/*
 * Filter function for directory entries
 * Input:   entry, directory entry
 * Output:  1,     if the entry is a directory and its name is only made by numbers
 * 			0,     else
 */
int filterDirectories(const struct dirent *entry)
{
	return entry->d_type == DT_DIR && isOnlyMadeByNumbers(entry->d_name);
}

/*
 * Get a soft link from a given file path
 * Input:   filePath, the file path
 * Output:  1,        if success
 * 			0,        else
 *
 * 			softLink, the soft link
 */
int getSoftLink(const char *filePath, char **softLink)
{
	int size, count;

	size = 64;
	do
	{
		*softLink = (char *) realloc(*softLink, size);
		count = readlink(filePath, *softLink, size);
		size *= 2;
	}
	while (count > size);

	return count > 0;
}

/*
 * Get current processes
 * Input:   None
 * Output:  Print PID and command for each running process
 */
static void getCurrentProcesses(void)
{
	int count, i;
	struct dirent **entries;
	char process[1024];
	char *softLink, *PATH;

	softLink = NULL;
	PATH = "/proc";
	count = scandir(PATH, &entries, filterDirectories, NULL);

	// Check if any errors occurred
	if (count < 0)
		errorAndDie("scandir");

	for (i = 0; i < count; i++)
	{
		snprintf(process, sizeof process, "%s%s%s%s", PATH, "/", entries[i]->d_name, "/exe");
		if (getSoftLink(process, &softLink))
			printf("%s %s\n", entries[i]->d_name, softLink);
	}
}

extern void run(int argc, char *argv[])
{
	getCurrentProcesses();
}

/*
 * Print error message and exit
 * Input: msg, the error message
 */
static inline void errorAndDie(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

/*
 * Print message and exit
 * Input: msg, the message
 */
static inline void printAndDie(const char *msg)
{
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}
