/*
Prova Pratica di Laboratorio di Sistemi Operativi
24 gennaio 2006
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2006-01-24.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline char *waitFifoIO(char *path);
static inline void sendSignals();

extern void run(int argc, char *argv[])
{
	if (argc != 1)
		printAndDie("Wrong number of parameters.\n");

	listenSignals();
}

/*
 * Wait for external input using a named pipe.
 * Input: path, the pipe path
 * Output: the input received
 */
static inline char *waitFifoIO(char *path)
{
    int *fifo;
    int ready;
    fd_set set;
    char *buffer[1024];

	// Clear the set
	FD_ZERO(&set);

	// Open the named pipe
	if ((fifo = open(path, O_RDWR | O_NONBLOCK)) < 0)
		errorAndDie("open");

	// Add the file descriptor to the set
	FD_SET(fifo, &set);

	// Block the program until I/O is ready
	if ((ready = select(fifo + 1, &set, NULL, NULL, NULL)) < 0)
		errorAndDie("select");

	if (read(fifo, &buffer, sizeof (buffer)) < 0)
		errorAndDie("read");

	return buffer;
}

static inline void sendSignals()
{
	int *signal = { SIGINT, SIGHUP, SIGALRM, SIGBUS, SIGUSR1 };
	char pid;

	pid = waitExternalOk("fifOK");

	printf("%s\n", pid);

	int i;
	for (i = 0; i < 5; i++)
		kill(atoi(pid), signal[i]);
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
