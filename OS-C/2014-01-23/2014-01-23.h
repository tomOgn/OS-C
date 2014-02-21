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
static int getEndOfLines(char *path, int theLine, int *lenght);

/*
 * Find a specific line in a text file and get its lenght (i.e. number of characters).
 * Input:
 * 			path,       the file path
 * 			theLine,    the line
 * Output:
 * 			lenght,     the lenght of the line
 *
 * 			1,          in case of success
 * 			0,          else
 */
static int getEndOfLines(char *path, int theLine, int *lenght)
{
	int c, count, line, output;
	FILE *stream;

	count = 0;
	line = 0;
	stream = fopen(path, "r");
	if (!stream)
	{
        perror("fopen");
        exit(EXIT_FAILURE);
	}

	while ((c = fgetc(stream)) != EOF && line < theLine)
	{
		if (c == '\n')
		{
			line++;
			if (line < theLine) count = 0;
		}
		else count++;
	}

	fclose(stream);

	if (line == theLine)
	{
		*lenght = count;
		output = TRUE;
	}
	else
	{
		*lenght = 0;
		output = FALSE;
	}

	return output;
}

/*
 * Check if a file is textual
 * Input:
 * 			fp, the file pointer
 * Output:
 * 			1, if the file is a text file
 * 			0, else
 */
static inline int isTextFile(char *path)
{
	int c;
	FILE *fp = fopen(path, "r");

	if (!fp)
	{
        perror("fopen");
        exit(EXIT_FAILURE);
	}

	while ((c = fgetc(fp)) != EOF && c >= 1 && c <= 127);
	fclose(fp);

	return c == EOF;
}

/*
 * Check if a file is hidden
 * Input:
 * 			fp, the file pointer
 * Output:
 * 			1, if the file is an hidden file
 * 			0, else
 */
static inline int isHiddenFile(FILE *fp)
{
	return fgetc(fp) == '.';
}

extern void run(char *path, int n)
{
	int count = 0;
	struct stat info;

	// Get file attributes
	if (!lstat(path, &info) < 0)
	{
        perror("lstat");
        exit(EXIT_FAILURE);
	}

	// If it is a text file and a regular file
	if (isTextFile(path) && S_ISREG(info.st_mode))
	{
		// Get the lenght of the n-th line
		if (getEndOfLines(path, n, &count))
		{
			printf("count = %d\n", count);
		}
	}

	exit(EXIT_SUCCESS);
}
