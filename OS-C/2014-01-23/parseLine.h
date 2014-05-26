/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 gennaio 2014
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2014.01.23.pdf

@author: Tommaso Ognibene
*/

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../const.h"

// Function declarations
static inline int isTextFile(char *path);
static inline int isHiddenFile(FILE *fp);
static inline int isRegularFile(char *path);
static int getLenghtOfLine(char *path, int theLine, int *lenght);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	int count, n;
	char *path;

	if (argc != 3)
		printAndDie("The functions require only 2 parameters to be passed in.");

	path = argv[1];
	n = atoi(argv[2]);
	count = 0;

	// If it is a text file and a regular file
	if (isTextFile(path) && isRegularFile(path))
	{
		// Get the lenght of the n-th line
		if (getLenghtOfLine(path, n, &count))
			printf("Lenght of line #%d = %d characters.\n",n, count);
		else
			printf("Line #%d does not exist.\n", n);
	}
}

/*
 * Find a specific line in a text file and get its lenght (i.e. number of characters).
 * Input:
 * 			path,       file path
 * 			theLine,    line number
 * Output:
 * 			lenght,     lenght of the line
 *
 * 			1,          in case of success
 * 			0,          else
 */
static int getLenghtOfLine(char *path, int theLine, int *lenght)
{
	int c, count, line, output;
	FILE *stream;

	count = 0;
	line = 0;
	stream = fopen(path, "r");
	if (!stream)
		errorAndDie("fopen");

	while ((c = fgetc(stream)) != EOF && line < theLine)
	{
		if (c == '\n')
		{
			line++;
			if (line < theLine)
				count = 0;
		}
		else
			count++;
	}

	fclose(stream);

	if (line == theLine)
	{
		*lenght = count;
		output = True;
	}
	else
	{
		*lenght = 0;
		output = False;
	}

	return output;
}

/*
 * Check if a file is textual
 * Input:
 * 			fp, file pointer
 * Output:
 * 			1,  if the file is a text file
 * 			0,  else
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
 * Check if a file is hidden
 * Input:
 * 			fp, file pointer
 * Output:
 * 			1,  if the file is an hidden file
 * 			0,  else
 */
static inline int isHiddenFile(FILE *fp)
{
	return fgetc(fp) == '.';
}

/*
 * Check if a file is regular
 * Input:
 * 			fp, file pointer
 * Output:
 * 			1,  if the file is an hidden file
 * 			0,  else
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
