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
#include <time.h>
#include <unistd.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void testPing(char *message, int times);

extern void run(int argc, char *argv[])
{
	if (argc != 1)
		printAndDie("The function does not require parameters to be passed in.");

	char message[40];
	memset(message, 'c', sizeof (message));
	message[39] = (char)0;

	testPing(message, 100000);
}

/*
 * Ping Test
 * Create a child process. Send a message n times and check the reply.
 * Count the overall elapsed time.
 * Input: message, the message to send
 *        times,   the n times
 */
static inline void testPing(char *message, int times)
{
	clock_t start, end;
	double elapsed;
	pid_t pid;
	int i;
	int fd[2][2];
	const int ToChild = 0, FromChild = 1, Lenght = strlen(message) + 1;
	char buffer[Lenght];

	// Create two pipes
	if (pipe(fd[ToChild]) < 0)
		errorAndDie("pipe");

	if (pipe(fd[FromChild]) < 0)
		errorAndDie("pipe");

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		// Close input side of pipe FromChild
		close(fd[FromChild][0]);

		// Close output side of pipe ToChild
		close(fd[ToChild][1]);

		while (1)
		{
			// Read on the pipe ToChild
			if (read(fd[ToChild][0], buffer, Lenght) < 0)
				errorAndDie("read");

			// Write on the pipe FromChild
			if (write(fd[FromChild][1], buffer, Lenght) < 0)
				errorAndDie("write");
		}
	}

	// Close input side of pipe ToChild
	close(fd[ToChild][0]);

	// Close output side of pipe FromChild
	close(fd[FromChild][1]);

	// Start time
	start = clock();

	// Ping Test
	for (i = 0; i < times; i++)
	{
		// Write on the pipe ToChild
		if (write(fd[ToChild][1], message, Lenght) < 0)
			errorAndDie("write");

		// Read on the pipe FromChild
		if (read(fd[FromChild][0], buffer, Lenght) < 0)
			errorAndDie("read");

		if (strcmp(message, buffer))
			printAndDie("Ping Test failed!\nThe child process has sent back a wrong message.");
	}

	// Stop time
	end = clock();

	// Kill child process
	kill(pid, SIGTERM);

	// Print elapsed time
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Ping Test succeeded!\n");
	printf("Test Message = %s\n", message);
	printf("Repetitions  = %d\n", times);
	printf("Elapsed Time = %0.5f\n", elapsed);
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
