/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 giugno 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.06.22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function declarations
static inline int isSignalNumber(int n);
void signalHandler(int signal);
static inline void runProcess(const char *path, char *command[]);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void sendSignal(pid_t target, int signal, int seconds, int count);

// Global variables
static int SignalNumber;
static char *Path;
static char **Command;

extern void run(int argc, char *argv[])
{
	int n, i;

	// Sanity checking
	if (argc < 3)
		printAndDie("The function requires at least two parameters to be passed in.");

	SignalNumber = atoi(argv[1]);
	if (!isSignalNumber(SignalNumber))
		printAndDie("The first parameter should be a valid signal number.");

	// Parse command
	Path = argv[2];
	n = argc - 2;
	Command = (char **) malloc((n + 1) * sizeof (char *));
	for (i = 0; i < n; i++)
		Command[i] = argv[i + 2];
	Command[i] = NULL;

	// Set signal handler
	if (signal(SignalNumber, &signalHandler) == SIG_ERR)
		errorAndDie("signal");

	// Test it
	sendSignal(getpid(), SignalNumber, 1, 10);

	// Everlasting busy waiting
    while (True);
}

/*
 * Check if a number is within the range of supported real-time signals.
 * Input:  n, number
 * Output: 1, if the number is within the range
 * 		   0, else
 */
static inline int isSignalNumber(int n)
{
	return n >= 0 && n <= 64;
}

void signalHandler(int signal)
{
	runProcess(Path, Command);
}

/*
 * Run a specific process.
 * Input:	path,    	the path to the file to run
 *          command,    the list of arguments
 * Output:	void
 */
static inline void runProcess(const char *path, char *command[])
{
	pid_t pid;

	// Fork process
	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Child
	if (pid == 0)
	{
		// Execute command
		execvp(path, command);
		errorAndDie("execv");
	}
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
	int n = 0;

	// Fork process
	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Child
	if (pid == 0)
	{
		while (n++ < count)
		{
			kill(target, signal);
			sleep(seconds);
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
