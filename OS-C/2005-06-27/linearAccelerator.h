/*
Prova Pratica di Laboratorio di Sistemi Operativi
27 giugno 2005
Esercizio 1 & 2

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.06.27.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
static inline void checkError(int error, const char *msg);
static inline void printAndDie(const char *msg);
static inline void linearAccelerator(int n);
static inline void terminateExperiment();

extern void run(int argc, char *argv[])
{
	// Sanity check
	if (argc != 2)
		printAndDie("The function requires only one paramenter to be passed in.");

	int n = atoi(argv[1]);
	if (n < 6)
		printAndDie("The number of nodes should be greater than 5.");

	linearAccelerator(n);
}

static inline void linearAccelerator(int n)
{
	pid_t pid;
	int value, i, particles, emitter, receiver;
	int fd[n][2];
	char particle[1] = { 'p' };
	time_t startTime, stopTime;

	// [1] Create the Linear Accelerator

	// Create the last pipe used as a trigger
	value = pipe(fd[n - 1]);
	checkError(value, "pipe");

	i = 0;
	do
	{
		// Create a pipe
		value = pipe(fd[i]);
		checkError(value, "pipe");

		// Fork the process
		pid = fork();
		checkError(pid, "fork");

		// Child
		if (pid == 0)
		{
			// Close output side
			value = close(fd[i][1]);
			checkError(value, "close");

			i++;
		}
		// Parent
		else
		{
			// Close input side
			value = close(fd[i][0]);
			checkError(value, "close");
		}
	} while (i < n - 1 && pid == 0);

	// [2] Trigger the Linear Accelerator
	if (i == n - 1)
	{
		// Close input side
		value = close(fd[n - 1][0]);
		checkError(value, "close");

		// The write is repeated if a signal has interfered
		do
			value = write(fd[n - 1][1], particle, 1);
		while (value < 0 && errno == EINTR);
		checkError(value, "write");

		// Close output side
		value = close(fd[n - 1][1]);
		checkError(value, "close");
	}
	else if (i == 0)
	{
		// Close output side
		value = close(fd[n - 1][1]);
		checkError(value, "close");

		// The read is repeated if a signal has interfered
		do
			value = read(fd[n - 1][0], particle, 1);
		while (value < 0 && errno == EINTR);
		checkError(value, "read");

		// Close input side
		value = close(fd[n - 1][0]);
		checkError(value, "close");
	}

	// [3] Execute the Linear Accelerator
	particles = 0;
	emitter = 0;
	receiver = n - 1;
	time(&startTime);
	while (True)
	{
		// Emitter
		if (i == emitter)
		{
			// The write is repeated if a signal has interfered
			do
				value = write(fd[i][1], particle, 1);
			while (value < 0 && errno == EINTR);

			// If there is only one node left, the experiment is terminated
			if (value < 0 && errno == EPIPE)
				terminateExperiment();

			checkError(value, "write");

			// Wait one second before firing another particle
			sleep(1);
		}
		// Transport
		else if (i < receiver)
		{
			// The read is repeated if a signal has interfered
			do
				value = read(fd[i - 1][0], particle, 1);
			while (value < 0 && errno == EINTR);
			checkError(value, "read");

			// If the previous node is dead, this node becomes the emitter
			if (value == 0)
				emitter = i;

			// 50% of chances to get the particle
			if (rand() % 2)
			{
				// The write is repeated if a signal has interfered
				do
					value = write(fd[i][1], particle, 1);
				while (value < 0 && errno == EINTR);

				// If the next node is dead, this node becomes the receiver
				if (value < 0 && errno == EPIPE)
				{
					receiver = i;

					// If there is only one node left, the experiment is terminated
					if (receiver == emitter)
						terminateExperiment();
				}
				else
					checkError(value, "write");
			}
		}
		// Receiver
		else
		{
			// The read is repeated if a signal has interfered
			do
				value = read(fd[i - 1][0], particle, 1);
			while (value < 0 && errno == EINTR);
			checkError(value, "read");

			// If there is only one node left, the experiment is terminated
			if (value == 0)
				terminateExperiment();

			time(&stopTime);
			particles++;
			printf("Particles that have been successfully received:\n");
			printf("\tTotal Number       = %d\n", particles);
			printf("\tAverage Per Second = %.2f\n", particles / difftime(stopTime, startTime));
		}
	}
}

static inline void terminateExperiment()
{
	printf("Emitter and receiver are the same node. The experiment is terminated.\n");
	exit(EXIT_SUCCESS);
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
