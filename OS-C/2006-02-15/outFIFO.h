/*
Prova Pratica di Laboratorio di Sistemi Operativi
15 febbraio 2006
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2006-02-15.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

// Constants
#define BUFFER 1024

// Function declarations
static inline int isFile(char *path);
static inline int max(int a[], int n);
static inline void checkError(int error, const char *msg);
static inline void printAndDie(const char *msg);
static inline void outputNamedPipe(char *name[], int n);

extern void run(int argc, char *argv[])
{
	// Sanity check
	if (argc == 1)
		printAndDie("Wrong number of parameters.");

	int i, value;
    struct stat info;
	char *name[argc];

	// Parse input
	for (i = 1; i < argc; i++)
	{
		// If the file does not exist
		if (!isFile(argv[i]))
		{
			// Create a named pipe
			value = mkfifo(argv[i], S_IRWXU);
			checkError(value, "mkfifo");
		}
		else
		{
			// Check if the file is a named pipe
			value = stat(argv[i], &info);
			checkError(value, "fstat");

			if (!S_ISFIFO(info.st_mode))
				printAndDie("Input error.");
		}
		name[i - 1] = argv[i];
	}

	outputNamedPipe(name, argc - 1);
}

/*
 * Retrieve the output of the given named pipes.
 * Input: name, the pipe names
 *        n,    the number of pipes
 */
static inline void outputNamedPipe(char *name[], int n)
{
    int *fifos;
    int ready, highest, buff_count, count, i, value;
    fd_set set, tmp;
    char buff[BUFFER];

	// Clear the set
	FD_ZERO(&set);

	// Populate the set
	fifos = (int *) malloc(sizeof (int) * n);
	for (i = 0; i < n; i++)
	{
		// Open the named pipe
		fifos[i] = open(name[i], O_RDWR | O_NONBLOCK);
		checkError(fifos[i], "open");

		// Add the file descriptor to the set
		FD_SET(fifos[i], &set);
	}

	// Get the highest-numbered file descriptor + 1
	highest = max(fifos, n) + 1;

	// Save a copy of the original set
	memcpy((void *) &tmp, (void *) &set, sizeof (fd_set));

	while (True)
	{
		// Block the program until I/O is ready on one or more of the file descriptors
		ready = select(highest, &set, NULL, NULL, NULL);
		checkError(ready, "select");

		for (i = 0, count = 0; count < ready || i < n; i++)
		{
			// Test if the file descriptor is part of the set
			if (!FD_ISSET(fifos[i], &set)) continue;

			count++;

			buff_count = read(fifos[i], &buff, BUFFER);
			checkError(buff_count, "read");

			value = write(STDOUT_FILENO, buff, sizeof (char) * buff_count);
			checkError(value, "write");
		}

		// Restore the original set
		memcpy((void *) &set,(void *) &tmp, sizeof (fd_set));
	}
}

/*
 * Check if a file exists.
 * Input:  path, file path
 * Output: 1,    if the file exists
 * 		   0,    else
 */
static inline int isFile(char *path)
{
	FILE *file = fopen(path, "r");
	if (file)
		fclose(file);

	return (file)? True: False;
}

/*
 * Find the max in an array.
 * Input:  a, an array
 *         n, its size
 * Output: the maximum value
 */
static inline int max(int a[], int n)
{
	int i, max = a[0];

	for (i = 1; i < n; i++)
		if (a[i] > max)
			max = a[i];

	return max;
}

/*
 * Check if the standard error code occured (return value < 0).
 * If yes, print the error message and exit.
 * Input: error, the return value of a system call
 *        msg,   the error message
 */
static inline void checkError(int error, const char *msg)
{
	if (error >= 0) return;

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
