/*
Prova Pratica di Laboratorio di Sistemi Operativi
15 febbraio 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.02.15.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Data types
typedef struct
{
	char *filePath;
	char *command[1024];
} Process;

// Function declarations
static inline void runProcess(Process *p);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void waitSignal(Process *process);
static inline Process *parseArguments(int argc, char *argv[]);
static inline void waitSignal(Process *process);

extern void run(int argc, char *argv[])
{
	Process *process;

	// Pre-conditions
	if (argc < 2)
		printAndDie("The function requires at least one parameter to be passed in.");

	// Parse arguments
	process = parseArguments(argc, argv);

	// Run child process and signal to the parent its return value
	waitSignal(process);
}

static inline Process *parseArguments(int argc, char *argv[])
{
	int i;
	Process *process = (Process *) malloc(sizeof(Process));

	process->filePath = argv[1];

	for (i = 0; i < argc - 1; i++)
		process->command[i] = argv[i + 1];
	process->command[i] = NULL;

	return process;
}

static inline void waitSignal(Process *process)
{
	int sigNum;
	sigset_t sigSet;

	// Initialize a signal set to full, including all signals
	if (sigfillset(&sigSet) < 0)
		errorAndDie("sigfillset");

	do
	{
		printf("/n1/n");
		runProcess(process);

		if (sigwait(&sigSet, &sigNum) < 0)
			errorAndDie("sigwait");
	}
	while (sigNum != 0);
}

/*
 * Run a specific process a signal the parent with the return value.
 * Input:	path,    	the path to the file to run
 *          command,    the list of arguments
 * Output:	void
 */
static inline void runProcess(Process *process)
{
	pid_t pid, parent;

	parent = getpid();
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		kill(parent, execvp(process->filePath, process->command));
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
