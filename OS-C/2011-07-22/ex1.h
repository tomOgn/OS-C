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
#include <openssl/hmac.h>
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
static inline int isExecutable2(char *path);
static void runConcurrentProcesses(char *paths[], char **argvs[], int n);
static int filterDirExecutable(const struct dirent *entry);

// Global variables
static char *cwd = ".";

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
 * Check if a file is executable
 * Input:  path, the file file
 * Output: 1, if the is executable
 *         0, else
 */
static inline int isExecutable2(char *path)
{
	struct stat buf;
	return ((stat(path, &buf) >= 0) && (buf.st_mode > 0) && (S_IXUSR & buf.st_mode));
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

	// Start processes
	for (i = 0; i < n; i++)
	{
		// Create a child process
		switch (pid = fork())
		{
		// Error
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		// Child process
		case 0:
			// Execute file
			exit(execvp(paths[i], argvs[i]));
		// Parent process
		}
	}

	// Wait processes
	for (i = 0; i < n; i++)
	{
		// Wait until a child process terminates
		if (wait(NULL) == -1)
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}

extern void run(int argc, char *argv[])
{
	struct dirent **fileNames;
	int count;
	int i;
	char **paths;
	char ***argvs;

	count = scandir(cwd, &fileNames, filterDirExecutable, NULL);
	if (count < 0)
	{
		perror("scandir");
		exit(EXIT_FAILURE);
	}

	paths = (char **) malloc(count * sizeof(char **));
	argvs = (char ***) malloc(count * sizeof(char ***));
	for (i = 0; i < count; i++)
	{
		paths[i] = (char *) malloc((sizeof(cwd) + sizeof(fileNames[i]->d_name) + 2) * sizeof(char *));
		sprintf(paths[i], "%s/%s", cwd, fileNames[i]->d_name);

		argvs[i] = (char **) malloc(2 * sizeof(char **));
		argvs[i][0] = fileNames[i]->d_name;
		argvs[i][1] = NULL;
	}

	runConcurrentProcesses(paths, argvs, count);

	exit(EXIT_SUCCESS);
}
