/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 luglio 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.07.22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Function declarations
static inline int isExecutable(char *path);
static void runConcurrentProcesses(char *paths[], char **argvs[], int n);
static int filterDirExecutable(const struct dirent *entry);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

// Global variables
static char *cwd = ".";

extern void run(int argc, char *argv[])
{
	struct dirent **file;
	int count, i;
	char **path;
	char ***args;

	count = scandir(cwd, &file, filterDirExecutable, NULL);
	if (count < 0)
		errorAndDie("scandir");

	path = (char **) malloc(count * sizeof(char *));
	args = (char ***) malloc(count * sizeof(char **));
	for (i = 0; i < count; i++)
	{
		path[i] = (char *) malloc(sizeof(cwd) + sizeof(file[i]->d_name) + 2 * sizeof(char));
		sprintf(path[i], "%s/%s", cwd, file[i]->d_name);

		args[i] = (char **) malloc(2 * sizeof(char *));
		args[i][0] = file[i]->d_name;
		args[i][1] = NULL;
	}

	runConcurrentProcesses(path, args, count);
}

/*
 * Filter function for Directories
 * Input:
 * 			entry, pointer to the directory entry
 * Output:
 * 			1, if the entry has .h or .c extension
 * 			0, else
 */
static int filterDirExecutable(const struct dirent *entry)
{
	char *path = (char *) malloc((sizeof(cwd) + sizeof(entry->d_name) + 2) * sizeof(char *));
	sprintf(path, "%s/%s", cwd, entry->d_name);

	if (!strcmp(entry->d_name, "OS-C")) return False;

	return ((entry->d_type == DT_REG || entry->d_type == DT_LNK) && isExecutable(path));
}

/*
 * Check if a file is executable
 * Input:  path, file path
 * Output: 1,    if it is executable
 * 		   0,    else
 */
static inline int isExecutable(char *path)
{
	return access(path, X_OK) == 0;
}

/*
 * Run an array of processes in a concurrent way.
 * Wait until all of them are terminated.
 * Input:	paths,	array of paths
 *          args,   array of array of arguments
 *          n,      dimension of the arrays
 * Output:	void
 */
static void runConcurrentProcesses(char *paths[], char **argvs[], int n)
{
	pid_t pid;
	int i;

	for (i = 0; i < n; i++)
	{
		// Fork
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		// Child
		if (pid == 0)
		{
			execvp(paths[i], argvs[i]);
			errorAndDie("execvp");
		}
	}

	// Wait until the children terminate
	for (i = 0; i < n; i++)
		if (wait(NULL) < 0)
			errorAndDie("waitpid");
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
