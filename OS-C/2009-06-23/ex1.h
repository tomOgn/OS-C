/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 giugno 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.06.23.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants


// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline int isTextFile(char *path);
static inline int isRegularFile(char *path);

extern void run(int argc, char *argv[])
{
	if (argc != 2)
		printAndDie("The function requires only one parameter to be passed in.\n");

	char *path = argv[1];
	if (!isRegularFile(path) && !isTextFile(path))
		printAndDie("The parameter should be a textual file.\n");


}

/*
 * Find a specific line in a text file and get its lenght (i.e. number of characters).
 * Inpstatic inline int isDirectory(char *path);ut:
 * 			path,       file path
 * 			theLine,    line number
 * Output:
 * 			lenght,     lenght of the line
 *
 * 			1,          in case of success
 * 			0,          else
 */
static void parseFileCommands(char *path, int theLine, int *lenght, FILE *st, char **command)
{
	int i, c, count, line, output, start;
	FILE *stream;
	char buf[1024];
	count = 0;
	line = 0;
	stream = fopen(path, "r");
	if (!stream)
		errorAndDie("fopen");

	c = fgetc(stream);
	while (c != EOF && line < theLine)
	{
		// Skip blank lines
		while (c == '\n' && c != EOF)
			c = fgetc(stream);

		// Skip comment lines
		while (c == '#')
		{
			c = fgetc(stream);
			while (c != '\n' && c != EOF)
				c = fgetc(stream);

			if (c != EOF)
				c = fgetc(stream);
		}
		line++;
		if (line == theLine)
		{
			i = 0;
			while (c != '\n' && c != EOF)
			{
				buf[i] = c;
				c = fgetc(stream);
			}
		}
		else
		{
			while (c != '\n' && c != EOF)
				c = fgetc(stream);
		}
	}
}

/*
 * Check if a file is regular
 * Input:  path, file path
 * Output: 1,    if the file is a regular file
 * 		   0,    else
 */
static inline int isRegularFile(char *path)
{
	struct stat info;

	// Get file attributes
	if (!lstat(path, &info) < 0)
		errorAndDie("lstat");

	return S_ISREG(info.st_mode);
}

/*
 * Check if a file is textual
 * Input:  path, file pointer
 * Output: 1,    if the file is a text file
 * 		   0,    else
 */
static inline int isTextFile(char *path)
{
	int c;
	FILE *fp = fopen(path, "r");

	if (!fp)
		errorAndDie("fopen");

	while ((c = fgetc(fp)) != EOF && c >= 1 && c <= 127);
	fclose(fp);

	return c == EOF;
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
