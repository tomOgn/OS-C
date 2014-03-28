/*
Prova Pratica di Laboratorio di Sistemi Operativi
17 luglio 2012
Esercizio 1 & 2

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.07.17.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../const.h"

// Constants
#define MaxLenArr 128 // Max length array
#define MaxLenStr 256 // Max length string

// Data types
typedef struct
{
	// File path
	char *pth;

	// Command
	char *cmd[MaxLenArr];

	// Environment variables
	char *env[2];
} Process;

// Function declarations
static inline Process *parseArguments(int argc, char **argv);
static inline void runProcess(Process *process, int n);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	// Pre-conditions
	if (argc < 3)
		printAndDie("The functions requires at least 2 parameters to be passed in.");

	// Number of copies
	int n = atoi(argv[1]);

	// Parse arguments
	Process *process = parseArguments(argc, argv);

	// Run n copies of the process re-directing the standard output
    runProcess(process, n);
}

/*
 * Parse command line arguments.
 * Input: argc, argument counter
 *        argv, argument vector
 * Output:
 *        Process, data structure containing:
 *           pth, file to run
 *           cmd, list of arguments
 *           env, environment variable
 */
static inline Process *parseArguments(int argc, char **argv)
{
	int i;
	Process *process = (Process *) malloc(sizeof (Process));

	// Set the file path
	process->pth = argv[2];

	// Set the argument list
	for (i = 0; i < argc - 2; i++)
		process->cmd[i] = argv[i + 2];
	process->cmd[i] = NULL;

	// Initialize the environment variable
	process->env[0] = (char *) malloc(sizeof (char) * MaxLenStr);
	process->env[1] = NULL;

	return process;
}

/*
 * Run n copies of a process.
 * Wait until all of them terminate.
 * Input: process, data structure containing:
 *           pth, file to run
 *           cmd, list of arguments
 *           env, environment variables
 *        n,       number of copies
 */
static inline void runProcess(Process *process, int n)
{
	pid_t pid;
	int started, terminated;
	char buf[4096];
	ssize_t count;
	int **fd;
	extern char **environ;

	// Initialize file descriptors to be used for the pipeline
	fd = (int **) malloc(sizeof (int **) * n);
	for (started = 0; started < n; started++)
		fd[started] = (int *) malloc (sizeof(int *) * 2);

	// For each copy
	for (started = 0; started < n; started++)
	{
		// Create a new pipe
		if (pipe(fd[started]) < 0)
			errorAndDie("pipe");

		// Fork
		pid = fork();
		if (pid < 0)
			errorAndDie("fork");

		// Child process
		if (pid == 0)
		{
			// Close input side of pipe
			close(fd[started][0]);

			// Re-direct the standard output into the pipe
			if (dup2(fd[started][1], STDOUT_FILENO) < 0)
				errorAndDie("dup2");

			// Set the environment variable
			sprintf(process->env[0], "%s=%d", "NCOPIA", started);
		    environ = process->env;

			// Execute command
			if (execvp(process->pth, process->cmd) < 0)
				errorAndDie("execve");

			exit(EXIT_SUCCESS);
		}

		// Close output side of pipe
		close(fd[started][1]);

		// Read on the pipe
		while ((count = read(fd[started][0], buf, sizeof(buf))) > 0)
		{
			if (write(1, buf, count) < 0)
				errorAndDie("write");

			if (memset(buf, 0, 10) < 0)
				errorAndDie("memset");
		}

		if (count < 0)
			errorAndDie("read");
	}

	// Wait processes
	for (terminated = 0; terminated < started; terminated++)
		if (wait(NULL) < 0)
			errorAndDie("wait");

	exit(EXIT_SUCCESS);
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
