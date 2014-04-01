/*
Prova Pratica di Laboratorio di Sistemi Operativi
03 febbraio 2010
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2010.02.03.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Constants
#define KB        1024
#define True      1
#define False     0
#define Success   TRUE
#define Failure   FALSE
#define MaxLenStr 512

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void printArrayR(char **x, int n);
static inline void printDirectory(char *path);
static int compareAlphabetic(const void *p1, const void *p2);

extern void run(int argc, char *argv[])
{
	// Pre-conditions
	if (argc != 1)
		printAndDie("The function does not require parameters to be passed in.");

	// List directory entries in reverse alphabetic order
	printDirectory(".");
}

/*
 * Print directory entries in reverse alphabetic order.
 * Input: path, path to the directory
 */
static inline void printDirectory(char *path)
{
	DIR *root;
	struct dirent *entry;
	char **file;
	int n, size;

	// Open directory
	root = opendir(path);
	if (!root)
		errorAndDie("opendir");

	// Initialize variables
	n = -1;
	size = 100 * sizeof (char *);
	file = (char **) malloc(size);
	if (!file)
		errorAndDie("malloc");

	// Get directory entries
	entry = readdir(root);
	while (entry)
	{
		// Adapt the array size to the number of entries
		n++;
		if (n == size)
		{
			size *= 2;
			file = (char **) realloc(file, size);
			if (!file)
				errorAndDie("realloc");
		}

		// Get the entry name
		file[n] = (char *) malloc(sizeof (char) * MaxLenStr);
		if (!file[n])
			errorAndDie("malloc");

		memset(file[n], 0, sizeof (char) * MaxLenStr);
		strcpy(file[n], entry->d_name);

		entry = readdir(root);
	}

	if (n == 0)
		printAndDie("The directory is empty.");

	// Sort alphabetically
	qsort(file, n, sizeof (char *), compareAlphabetic);

	// Print in reverse order
	printArrayR(file, n);
}

/*
 * Comparison function to sort alphabetically.
 * Input:  s1, sentence 1
 *         s2, sentence 2
 * Output: 1,  if sentence 1 has a greater value than sentence 2
 *         0,  else
 */
static int compareAlphabetic(const void *s1, const void *s2)
{
    const char **i1 = (const char **)s1;
    const char **i2 = (const char **)s2;

    return strcmp(*i1, *i2);
}

/*
 * Print the elements of an array in reverse order.
 * Input:  x, array
 *         n, array size
 */
static inline void printArrayR(char **x, int n)
{
	int i;

	for (i = n - 1; i >= 0; i--)
		printf("%s\n", x[i]);
	printf("\n");
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
