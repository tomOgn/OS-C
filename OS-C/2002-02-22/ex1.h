/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 02 2002
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2002-02-22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

extern void run(int argc, char *argv[])
{
	if (argc < 2)
		printAndDie("The function requires at least one parameter to be passed in.");


}

static void runProcess(char *path, char *command[], int n)
{
	pid_t pid, next;
	int i, count;
	int fd[n][2];
	char buffer[32];

	// Create n encapsulated processes
	i = 1;
	do
	{
		// Create a new pipe
		if (pipe(fd[i]) < 0)
			errorAndDie("pipe");

		pid = fork();
		if (pid < 0)
			errorAndDie("fork");

		if (pid == 0)
		{
			// Close input side of pipe
			close(fd[i][0]);

			// Write on the pipe
			sprintf(buffer, "%d", (int) getpid());
			if (write(fd[i][1], buffer, 32) < 0)
				errorAndDie("write");
		}
		i++;
	}
	while (pid == 0 && i < n);

	// Close output side of pipe
	close(fd[i][1]);

	// Read on the pipe
	count = read(fd[i][0], buffer, sizeof (buffer));
	if (count < 0)
		errorAndDie("read");




	// Wait until the child process terminates
	if (wait(NULL) < 0)
		errorAndDie("waitpid");

	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		if (execvp(path, command) < 0)
			errorAndDie("execvp");

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
