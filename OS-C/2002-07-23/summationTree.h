/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 luglio 2002
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2002-07-23.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <limits.h>

// Constants
#define BUFFER PIPE_BUF

// Function declarations
static inline int max(int a[], int n);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static int openFIFO(char *path);

extern void run(int argc, char *argv[])
{
    int height;
	int *fifos;
    int ready_fd, nfds, buff_count, count, i;
    fd_set fdset, tmpset;
    char buff[BUFFER];
    struct stat buf_fstat;
    pid_t pid;
    char *path, *command;
    int fd[2][2];
	if (argc != 2)
		printAndDie("Wrong number of parameters.");

	height = argv[1];

	while (height > 1 && pid == 0)
	{
		// Create left and right pipe
		if (pipe(fd[0]) < 0 || pipe(fd[1]) < 0)
			errorAndDie("pipe");

		// Close input side
		if ((close(fd[0][0]) < 0) || (close(fd[1][0]) < 0))
			errorAndDie("close");

		// Fork
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		// Left process
		if (pid == 0)
		{
			height--;

			// Close input side
			if ((close(fd[0][0]) < 0) || (close(fd[1][0]) < 0))
				errorAndDie("close");


		}
		else
		{
			if ((pid = fork()) < 0)
				errorAndDie("fork");
			// Child process
			if (pid == 0)
				height--;
		}
	}

	FD_ZERO(&fdset);
	fifos = (int *) malloc(sizeof(int) * (argc - 1));
	for (i = 0; i < argc - 1; i++)
	{
		if ((fifos[i] = open(argv[i + 1], O_RDWR | O_NONBLOCK)) < 0)
			errorAndDie("open");

		if (fstat(fifos[i], &buf_fstat))
			errorAndDie("fstat");

		if (!S_ISFIFO(buf_fstat.st_mode))
			printAndDie(sprintf("%s is not a pipe.", argv[i + 1]));

		FD_SET(fifos[i], &fdset);
	}

	nfds = max(fifos, (argc - 1)) + 1;
	memcpy((void *) &tmpset, (void *) &fdset, sizeof(fd_set));

	while (1)
	{
		if ((ready_fd = select(nfds, &fdset, NULL, NULL, NULL)) < 0)
			errorAndDie("select");

		printf("ready fd: %d\n", ready_fd);
		for (i = 0, count = 0; count < ready_fd || i <= (argc - 1); i++)
		{
			if (FD_ISSET(fifos[i],&fdset))
			{
				count++;
				printf("fifo: %d\n", i);
				if ((buff_count = read(fifos[i], &buff, BUFFER)) < 0)
					errorAndDie("read");

				printf("read %d bytes\n", buff_count);
				if (write(STDOUT_FILENO, buff, sizeof(char) * (unsigned int) buff_count) < 0)
					errorAndDie("write");
			}
		}
		memcpy((void *) &fdset,(void *) &tmpset, sizeof(fd_set));
	}
}

static int openFIFO(char *path)
{
	int output;
	struct stat buf_fstat;

	if ((output = open(path, O_RDWR| O_NONBLOCK)) < 0)
		errorAndDie("open");

	if (fstat(output, &buf_fstat))
		errorAndDie("fstat");

	if (!S_ISFIFO(buf_fstat.st_mode))
		printAndDie(sprintf("%s is not a pipe.", path));

	return output;
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
