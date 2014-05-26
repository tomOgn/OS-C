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
#include <sys/wait.h>

// Data types
typedef struct
{
	char *filePath;
	char *command[1024];
} Process;

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void runAndRepeatOnError(Process *process);
static inline Process *parseArguments(int argc, char *argv[]);

extern void run(int argc, char *argv[])
{
	Process *process;

	// Pre-conditions
	if (argc < 2)
		printAndDie("The function requires at least one parameter to be passed in.");

	// Parse arguments
	process = parseArguments(argc, argv);

	// Run child process and signal to the parent its return value
	runAndRepeatOnError(process);
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

/*
 * Run a process. Repeat on error.
 */
static void runAndRepeatOnError(Process *process)
{
	pid_t pid;
	int status;

	do
	{
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		if (pid == 0)
		{
			execvp(process->filePath, process->command);
			errorAndDie("execvp");
		}

		if (wait(&status) < 0)
			errorAndDie("waitpid");
	}
	while (status != 0);
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
