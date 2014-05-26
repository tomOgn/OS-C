/*
Prova Pratica di Laboratorio di Sistemi Operativi
15 gennaio 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.01.15.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

static void runEndlessProcess(char *path, char *command[]);

extern void run(int argc, char *argv[])
{
	if (argc < 2)
		printAndDie("The function requires at least one parameter to be passed in.");

	// File path
	char *path = argv[2];

	// Number of arguments
	int n = argc - 2;

	// Arguments
    char **command = (char **) malloc(sizeof (char *) * (n + 1));
    int i;
    for (i = 0; i < n; i++)
    	command[i] = argv[i + 2];
    command[i] = NULL;

    runEndlessProcess(path, command);
}

static void runEndlessProcess(char *path, char *command[])
{
	pid_t pid;

	while (True)
	{
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		// Child process
		if (pid == 0)
		{
			execvp(path, command);
			errorAndDie("execvp");
		}

		// Wait until the child process terminates
		if (wait(NULL) < 0)
			errorAndDie("waitpid");
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
