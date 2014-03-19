/*
Prova Pratica di Laboratorio di Sistemi Operativi
17 luglio 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.07.17.pdf

@author: Tommaso Ognibene
*/

// Pre-processor directives
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../const.h"
#include <sys/wait.h>

/*
 * Read characters from the pipe and echo them to stdout.
 */
void readPipe(int file)
{
	FILE *stream;
	int c;
	stream = fdopen(file, "r");
	while ((c = fgetc(stream)) != EOF)
		putchar(c);
	fclose(stream);
}

/*
 * Write some random text to the pipe.
 */
void writePipe(int file)
{
	FILE *stream;
	stream = fdopen(file, "w");
	fprintf(stream, "hello, world!\n");
	fprintf(stream, "goodbye, world!\n");
	fclose(stream);
}

int tryPipe(void)
{
	pid_t pid;
	int mypipe[2];

	/* Create the pipe. */
	if (pipe(mypipe))
	{
		fprintf (stderr, "Pipe failed.\n");
		return EXIT_FAILURE;
	}

	/* Create the child process. */
	pid = fork();
	if (pid == (pid_t)0)
	{
		/* This is the child process. Close other end first. */
		close(mypipe[0]);
		writePipe(mypipe[1]);
		return EXIT_SUCCESS;
	}
	else if (pid < (pid_t)0)
	{
		/* The fork failed. */
		fprintf(stderr, "Fork failed.\n");
		return EXIT_FAILURE;
	}
	else
	{

		/* This is the parent process. Close other end first. */
		close(mypipe[1]);
		readPipe(mypipe[0]);

		return EXIT_SUCCESS;
	}
}

void tryPipeSTDOUT(void)
{
	pid_t pid;
	int p[2];

	pipe(p);
	pid = fork();
	if (pid == 0)
	{
		dup2(p[1], 1); // redirect the output (STDOUT to the pipe)
		close(p[0]);
		execlp("cat", "cat", NULL);
		exit(EXIT_FAILURE);
	}
	else
	{
		close(p[1]);
		fd_set rfds;
		char buffer[10] = {0};
		while (1)
		{
			FD_ZERO(&rfds);
			FD_SET(p[0], &rfds);
			select(p[0] + 1, &rfds, NULL, NULL, NULL); //wait for changes on p[0]
			if (FD_ISSET(p[0], &rfds))
			{
				int ret = 0;
				while ((ret = read(p[0], buffer, 10)) > 0) //read on the pipe
				{
					write(1, buffer, ret); //display the result
					memset(buffer, 0, 10);
				}
			}
		}
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

/*
 * Run n copies of a process in a concurrent way.
 * Wait until all of them are terminated.
 * Input:	command, the process to run
 *          args,    the list of arguments
 *          env,     the environment variables
 *          n,       the number of copies
 * Output:	void
 */
void runProcessCopies(char *path, char **args, char *env[], int n)
{
	pid_t pid;
	int status, terminated = 0, started;
	char buf[4096];
	ssize_t count;
	// n pipes
	int **fd = (int **)malloc(sizeof(int **) * n);

	for (started = 0; started < n; started++)
	{
		fd[started] = (int *)malloc(sizeof(int *) * 2);
	}

	// For each copy
	for (started = 0; started < n; started++)
	{
		// Create a new pipe
		if (pipe(fd[started]) == -1)
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		// Create child process
		switch (pid = fork())
		{
		// Error
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		// Child process
		case 0:
			// Re-direct the standard output of the child process into the pipe
			if (dup2(fd[started][1], STDOUT_FILENO) == -1)
			{
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(fd[started][0]);
			close(fd[started][1]);

			// Create environment variable
			sprintf(env[0], "%s=%d", "NCOPIA", started);

			// Execute command
			exit(execve(path, args, env));

		// Parent process
		default:
			close(fd[started][1]);

			// Read on the pipe
			while ((count = read(fd[started][0], buf, sizeof(buf))) > 0)
			{
				write(1, buf, count);
				memset(buf, 0, 10);
			}

			if (count == -1)
			{
				perror("read");
				exit(EXIT_FAILURE);
			}
		}
	}

	// Wait processes
	for (terminated = 0; terminated < started; terminated++)
	{
		// Wait until a child process terminates
		if (wait(&status) == -1)
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}

void run(int argc, char *argv[])
{
	// Number of copies
	int n = atoi(argv[1]);
	// File path
	char *path = argv[2];
	// Number of arguments
	int m = argc - 2;
	// Arguments
    char **args = (char **)malloc(sizeof(char*) * (m + 1));
    // Environment variables
    char *env[2];
    env[0] = (char *)malloc(sizeof(char*) * 100);
    env[1] = NULL;

    int i;
    for (i = 0; i < m; i++)
    {
    	args[i] = argv[i + 2];
    }
    args[i] = NULL;

    runConcurrentProcessCopies(path, args, env, n);
}
