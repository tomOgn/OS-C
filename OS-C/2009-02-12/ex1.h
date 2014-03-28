/*
Prova Pratica di Laboratorio di Sistemi Operativi
12 febbraio 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.02.12.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void parseArguments(int argc, char **argv, char **path, char ***command);
static inline void runProcess(char *path, char *command[]);

extern void run(int argc, char *argv[])
{
	if (argc < 2)
		printAndDie("The function requires at least one parameter to be passed in.");

	char *path;
	char **command;

	// Parse arguments
	parseArguments(argc, argv, &path, &command);

	// Run process
	runProcess(path, command);
}

/*
 * Parse command line arguments.
 * Input:  argc,    argument counter
 *         argv,    argument vector
 * Output: path,    file to run
 *         command, list of arguments
*/
static inline void parseArguments(int argc, char **argv, char **path, char ***command)
{
	int i;

	(*command) = (char **) malloc(sizeof (char *) * argc);

	// Set the file path
	(*path) = argv[1];

	// Set the argument list
	(*command)[0] = argv[1];
	for (i = argc - 1; i > 1; i--)
		// Invert parameters
		(*command)[argc - i] = argv[i];
	(*command)[argc - 1] = NULL;
}

/*
 * Run a process.
 * Wait until it terminates.
 * Input: path,    file to run
 *        command, list of arguments
 */
static inline void runProcess(char *path, char *command[])
{
	pid_t pid;

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		// Execute command
		if (execvp(path, command) < 0)
			errorAndDie("execve");

		exit(EXIT_SUCCESS);
	}

	// Wait until it terminates
	if (wait(NULL) < 0)
		errorAndDie("wait");
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
