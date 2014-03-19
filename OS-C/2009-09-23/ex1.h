/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 settembre 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.09.23.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants
#define KB 1024

// Function declarations
static inline int getSizeFile(char *path, off_t *size);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void inverseArray(int x[], int n);
static inline void printArray(int x[], int n);

extern void run(int argc, char *argv[])
{
	off_t count, size, offset;
	char *pathSrc, *pathDst;
	void *buf;
	int fdSrc, fdDst;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	if (argc != 3)
		printAndDie("The function requires two parameters to be passed in.\n");

	pathSrc = argv[1];
	pathDst = argv[2];
	if (!getSizeFile(pathSrc, &size))
		printAndDie("The first parameter should be an existing file.\n");

	fdSrc = open(pathSrc, O_RDONLY);
	if (fdSrc < 0)
		errorAndDie("open");

	fdDst = open(pathDst, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (fdDst < 0)
		errorAndDie("open");

	count = 4 * KB;
	buf = (void *) malloc(count * sizeof(void *));
	for (offset = 0; offset + count <= size; offset += count)
	{
		if (pread(fdSrc, buf, count, offset) < 0)
			errorAndDie("pread");

		//printf("First element = %d\n", ((int *)buf)[0]);

		inverseArray(buf, count);
		if (pwrite(fdDst, buf, count, size - offset - count) < 0)
			errorAndDie("pread");
	}

	if (offset != size)
	{
		if (pread(fdSrc, buf, size - offset, offset) < 0)
			errorAndDie("pread");

		//printf("Last element = %d\n", ((int *)buf)[size - offset - 1]);

		if (pwrite(fdDst, buf, size - offset, 0) < 0)
			errorAndDie("pread");
	}
}

/*
 * Print the elements of an array.
 * Input:  x, array
 *         n, array size
 */
static inline void printArray(int x[], int n)
{
	int i;

	for(i = 0; i < n; i++)
		printf("%d ", x[i]);
	printf("\n");
}

/*
 * Invert the elements of an array.
 * Input:  x, array
 *         n, array size
 */
static inline void inverseArray(int x[], int n)
{
	int i, tmp;

	n--;
	for(i = 0; i < n / 2; i++)
	{
		tmp = x[i];
		x[i] = x[n];
		x[n] = tmp;
		n--;
	}
}

/*
 * Check if a file is a regular file.
 * If yes, return its size in Bytes.
 * Input:  path, file path
 * Output: 1,    if it is a regular file
 * 		   0,    else
 *
 * 		   size, its size
 */
static inline int getSizeFile(char *path, off_t *size)
{
	struct stat info;
	int output = FALSE;

	if (stat(path, &info) < 0)
		errorAndDie("stat");

	if (info.st_mode & S_IFREG)
	{
		output = TRUE;
		*size = info.st_size;
	}

	return output;
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
