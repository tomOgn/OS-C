/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 settembre 2005
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.09.20.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	if (argc != 2)
		printAndDie("The function requires only one parameter to be passed in.\n");

	int pid = atoi(argv[1]);
	if (!isProcess(pid))
		printAndDie("The parameter should be a running process.\n");

	signalRepeater(pid);
}

/*
 * Send signals to a running process.
 * Input:  pid,   the running process
 */
static inline void signalRepeater(int pid)
{
	int sigNum, i;
	sigset_t sigSet;
	int *terminatingSignals = { SIGTERM, SIGINT, SIGQUIT, SIGKILL, SIGHUP };

	// Initialize and empty a signal set
	if (sigemptyset(&sigSet) < 0)
		errorAndDie("sigemptyset");

	// Initialize a signal set to full, including all signals
	if (sigfillset(&sigSet) < 0)
		errorAndDie("sigfillset");

	// Remove terminating signals
	for (i = 0; i < 5; i++)
		if (sigdelset(&sigSet, terminatingSignals[i++]) < 0)
			errorAndDie("sigdelset");

	while (TRUE)
	{
		if (sigwait(&sigSet, &sigNum) < 0)
			errorAndDie("sigwait");

		kill(pid, sigNum);
	}
}

/*
 * Check whether a PID refers to a running process or not.
 * Input:  pid,   the PID to check
 * Output: TRUE,  if yes
 *         FALSE, otherwise
 */
static inline int isProcess(int pid)
{
	return !kill(pid, 0);
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
