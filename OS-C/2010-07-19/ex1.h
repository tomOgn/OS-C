/*
Prova Pratica di Laboratorio di Sistemi Operativi
19 luglio 2010
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2010-07-19.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Constants

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void pingProcess(char *message, int times);

extern void run(int argc, char *argv[])
{
	if (argc != 1)
		printAndDie("The function does not require parameters to be passed in.");

	char message[40];
	memset(message, 'c', sizeof (message));

	printf("size = %d\n", sizeof (message));

	pingProcess(message, 100000);
}

static inline void pingProcess(char *message, int times)
{
	pid_t pid;
	int i;
	int fd[2][2];
	const int toChild = 0, fromChild = 1;
	char buffer[sizeof (message)];

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Create two pipes
	if (pipe(fd[toChild]) < 0)
		errorAndDie("pipe");

	if (pipe(fd[fromChild]) < 0)
		errorAndDie("pipe");

	// Child process
	if (pid == 0)
	{
		// Close input side of pipe fromChild
		close(fd[fromChild][0]);

		// Close output side of pipe toChild
		close(fd[toChild][1]);

		while (1)
		{
			// Read on the pipe toChild
			if (read(fd[toChild][0], buffer, sizeof (buffer)) < 0)
				errorAndDie("read");

			// Write on the pipe fromChild
			if (write(fd[fromChild][1], buffer, sizeof (buffer)) < 0)
				errorAndDie("write");
		}
	}

	// Close input side of pipe toChild
	close(fd[toChild][0]);

	// Close output side of pipe fromChild
	close(fd[fromChild][1]);

	for (i = 0; i < times; i++)
	{
		// Write on the pipe toChild
		if (write(fd[toChild][1], message, sizeof (message)) < 0)
			errorAndDie("write");

		// Read on the pipe fromChild
		if (read(fd[fromChild][0], buffer, sizeof (buffer)) < 0)
			errorAndDie("read");

		if (strcmp(message, buffer))
			printAndDie("The child process has sent a wrong message.");
	}
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
