/*
Prova Pratica di Laboratorio di Sistemi Operativi
15 febbraio 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.02.15.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Data types
typedef struct
{
	char *filePath;
	char *command[1024];
} process;

// Function declarations
inline void runProcessAndReturnValue(const char *path, char *command[]);
static inline void errorAndDie(const char *msg);
static inline void waitSignalAndRepeatIfFailure(process *p);
static inline process *getProcessInput(int argc, char *argv[]);

/*
 * Print error message and exit
 * Input: msg, the error message
 */
static inline void errorAndDie(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

static inline void waitSignalAndRepeatIfFailure(process *p)
{
	int sigNumber;

	sigset_t sigSet;
	sigemptyset(&sigSet);
	sigfillset(&sigSet);

	while (TRUE)
	{
		sigwait (&sigSet, &sigNumber);

		if (sigNumber == 0) exit(EXIT_SUCCESS);
		else runProcessAndReturnValue(p->filePath, p->command);
	}
}

static inline process *getProcessInput(int argc, char *argv[])
{
	int n, i;
	process *p = (process *) malloc(sizeof(process));

	n = argc - 1;
	p->filePath = argv[1];
	//p->command = (char **) malloc((n + 1) * sizeof(char **));
	for (i = 0; i < n; i++)
		p->command[i] = argv[i + 1];
	p->command[i] = NULL;

	return p;
}

extern void run(int argc, char *argv[])
{
	process *p;

	// Pre-conditions
	if (argc < 2)
	{
		printf("The function requires at least one parameter to be passed in.\n");
		exit(EXIT_FAILURE);
	}

	p = getProcessInput(argc, argv);
	runProcessAndReturnValue(p->filePath, p->command);
	waitSignalAndRepeatIfFailure(p);
}

/*
 * Run a specific process.
 * Input:	path,    	the path to the file to run
 *          command,    the list of arguments
 * Output:	void
 */
inline void runProcessAndReturnValue(const char *path, char *command[])
{
	pid_t pid, parent;

	parent = getpid();

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
		kill(parent, execvp(path, command));
	}
}
