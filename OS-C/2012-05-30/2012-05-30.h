/*
Prova Pratica di Laboratorio di Sistemi Operativi
15 febbraio 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.05.30.pdf

@author: Tommaso Ognibene
*/

// Pre-processor directives
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

static const char *DELIMITER = ":";
static const char *MINI_MAKE_FILE = "/minimakefile";

static inline int countTokens(const char *sentence)
{
	char tmp[strlen(sentence)];
	int count = 0;
	char *token;

	strcpy(tmp, sentence);
	token = strtok(tmp, " ");

	while (token)
	{
		count++;
		token = strtok(NULL, " ");
	}

	return count;
}

static inline char **tokenize(const char *sentence)
{
	char tmp[strlen(sentence)];
	strcpy(tmp, sentence);
	int i, count = countTokens(sentence);
	char **tokens = (char **) malloc(sizeof(char **) * (count + 1));

	tokens[0] = strtok(tmp, " ");
	for (i = 1; i < count; i++)
	{
		tokens[i] = strtok(NULL, " ");
	}

	tokens[count] = NULL;

	return tokens;
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
 * Run a specific process.
 * Wait until it terminates.
 * Input:	path,    the path to the file to run
 *          args,    the list of arguments
 * Output:	void
 */
static void runProcess(const char *path, char *argv[])
{
	pid_t pid;

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
		if (execvp(path, argv) == -1)
			errorAndDie("execvp");
		break;
	// Parent process
	}

	// Wait until the child process terminates
	if (wait(NULL) < 0)
		errorAndDie("waitpid");
}

/*
 * Run a specific process.
 * Wait until it terminates.
 * Input:	path,    the path to the file to run
 *          args,    the list of arguments
 * Output:	void
 */
static inline int fileExists(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp)
	{
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

static inline void findPointer(const char *line, const char *cwd)
{
	char tmp[strlen(line)];
	char path[1024];
	char *target, *command;
	char **argv;

	strcpy(tmp, line);
	target = strtok(tmp, DELIMITER);
	command = strtok(NULL, DELIMITER);

	strcpy(path, cwd);
	strcat(path, "/");
	strcat(path, target);
	printf("path = %s\n", path);
	if (!fileExists(path))
	{
		argv = tokenize(command);
		strcpy(path, argv[0]);
		printf("path = %s\n", path);
		runProcess(path, argv);
	}
}

extern void run(int argc, char *argv[])
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char cwd[1024];
	char path[1024];

	// Get current working directory
	if (!getcwd(cwd, sizeof(cwd)))
	{
		perror("getcwd");
		exit(EXIT_FAILURE);
	}

	// Open the minimakefile
	strcpy(path, cwd);
	strcat(path, MINI_MAKE_FILE);
	fp = fopen(path, "r");
	if (!fp)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	// Read the file line by line
	while ((read = getline(&line, &len, fp)) != -1)
	{
		findPointer(line, cwd);
	}

	exit(EXIT_SUCCESS);
}
