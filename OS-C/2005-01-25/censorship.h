/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 gennaio 2005
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.01.25.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stddef.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void censorship(char *path, char *command[], char *forbiddenWord);
static char *replaceSubString(const char *text, const char *oldString, const char *newString);

extern void run(int argc, char *argv[])
{
	char *path, *forbiddenWord;
	char *command[argc - 1];
	int i;

    // Sanity check
	if (argc < 3)
		printAndDie("The function requires at least 2 parameters to be passed in.");

	// Parse parameters
	forbiddenWord = argv[1];
	path = argv[2];
	for (i = 0; i < argc - 2; i++)
		command[i] = argv[i + 2];
	command[i] = NULL;

	// Perform censorship
	censorship(path, command, forbiddenWord);
}

static void censorship(char *path, char *command[], char *forbiddenWord)
{
	pid_t pid;
	int fd[2];
	char buffer[8192];
	char *text;
	ssize_t count;

	// Fork
	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Create a pipe for output redirection
	if (pipe(fd) < 0)
		errorAndDie("pipe");

	// Child process
	if (pid == 0)
	{
		// Close input side
		if (close(fd[0]) < 0)
			errorAndDie("close");

		// Re-direct the standard output
		if (dup2(fd[1], STDOUT_FILENO) < 0)
			errorAndDie("dup2");

		execvp(path, command);
		errorAndDie("execve");
	}

	// Close output side
	if (close(fd[1]) < 0)
		errorAndDie("close");

	// Wait termination
	if (wait(NULL) < 0)
		errorAndDie("wait");

	// Clear the buffer
	memset(buffer, 0, sizeof (buffer));

	// Read from the buffer
	count = read(fd[0], buffer, sizeof (buffer));
	if (count < 0)
		errorAndDie("read");

	printf("%s", buffer);
	//text = replaceSubString(buffer, forbiddenWord, "");
	//write(STDOUT_FILENO, buffer, sizeof(buffer));
}

static char *replaceSubString(const char *text, const char *oldString, const char *newString)
{
	char *ret, *r;
	const char *i, *q;
	size_t oldLength = strlen(oldString), count, retLength, newLength = strlen(newString);

	if (!text || !oldString || !newString || oldLength == 0)
		return NULL;

	if (oldLength != newLength)
	{
		count = 0;
		for (i = text; (q = strstr(i, oldString)); i = q + oldLength)
			count++;

		retLength = i - text + strlen(i) + count * (newLength - oldLength);
	}
	else
		retLength = strlen(text);

	if (!(ret = malloc(retLength + 1)))
		return NULL;

	for (r = ret, i = text; (q = strstr(i, oldString)); i = q + oldLength)
	{
		ptrdiff_t l = q - i;
		memcpy(r, i, l);
		r += l;
		memcpy(r, newString, newLength);
		r += newLength;
	}
	strcpy(r, i);

	return ret;
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
