/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 giugno 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.06.22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Function declarations
static inline int isNatural(int n);
static inline int isSignalNumber(int n);
void signalHandlerRunProcess(int signal);
void signalHandlerTerminate(int signal);
static void runProcess(const char *path, char *command[]);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void sendSignal(pid_t target, int signal, int seconds, int count);

// Global variables
static int signalNumber;
static char *path;
static char **command;

/*
 * Check if a number is natural.
 * Input:  n, number
 * Output: 1, if the number is natural
 * 		   0, else
 */
static inline int isNatural(int n)
{
	return n >= 0 && n == floor(n);
}

/*
 * Check if a number is within the range of supported real-time signals.
 * Input:  n, number
 * Output: 1, if the number is within the range
 * 		   0, else
 */
static inline int isSignalNumber(int n)
{
	return isNatural(n) && n >= 0 && n <= 64;
}

extern void run(int argc, char *argv[])
{
	int n, i;

	if (argc < 3)
		printAndDie("The function requires at least two parameters to be passed in.\n");

	signalNumber = atoi(argv[1]);
	if (!isSignalNumber(signalNumber))
		printAndDie("The first parameter must be a valid signal number.\n");

	path = argv[2];
	n = argc - 2;
	command = (char **) malloc((n + 1) * sizeof(char **));
	for (i = 0; i < n; i++)
		command[i] = argv[i + 2];
	command[i] = NULL;

	// Set signal handlers
	signal(signalNumber, &signalHandlerRunProcess);

	// Test it
	sendSignal(getpid(), signalNumber, 1, 10);

	// Busy waiting
    while (TRUE);
}

void signalHandlerRunProcess(int signal)
{
	runProcess(path, command);
}

/*
 * Run a specific process.
 * Input:	path,    	the path to the file to run
 *          command,    the list of arguments
 * Output:	void
 */
static void runProcess(const char *path, char *command[])
{
	pid_t pid;

	// Create child process
	switch (pid = fork())
	{
	// Error
	case -1:
		errorAndDie("fork");
		break;
	// Child process
	case 0:
		// Execute command
		execvp(path, command);
		errorAndDie("execv");
	}

	exit(EXIT_SUCCESS);
}

/*
 * Create a child process. The child process sends a signal to a
 * process n times every m seconds.
 * Input:  target,  process target
 *         signal,  signal to send
 *         seconds, interval of seconds
 *         count,   times
 */
static inline void sendSignal(pid_t target, int signal, int seconds, int count)
{
	pid_t pid;
	int n = 1;

	// Create child process
	switch (pid = fork())
	{
	// Error
	case -1:
		errorAndDie("fork");
		break;
	// Child process
	case 0:
		// Execute command
		while (n <= count)
		{
			printf("I send the signal!\n");
			sleep(seconds);
			kill(target, signal);
			n++;
		}
		exit(EXIT_SUCCESS);
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
