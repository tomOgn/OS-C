/*
Prova Pratica di Laboratorio di Sistemi Operativi
30 maggio 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.05.30.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function declarations
static void parseMakeFile(char *path);
static inline int isFile(char *path);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void runProcess(char *path, char *argv[]);
static int countTokens(const char *sentence, int *longest);
static int tokenize(char *sentence, char ***tokens);
static inline int isEnd(int c);
static inline int isSpace(int c);

extern void run(int argc, char *argv[])
{
	parseMakeFile("minimakefile");
}

/*
 * Parse a mini MakeFile. Extract and run the commands.
 * Input:  path, path of the mini MakeFile
 */
static void parseMakeFile(char *path)
{
	char *target, *command, *line, *delimiter = ":";
	char **tokens;
	FILE *file;
    size_t len = 0;

	// Open the file
    file = fopen(path, "r");
	if (!file)
		printAndDie("Mini Make File not found.");

	// Parse the file line by line
	while (getline(&line, &len, file) != -1)
	{
		// Extract target and command
		target = strtok(line, delimiter);
		command = strtok(NULL, delimiter);

		// Tokenize and execute target
		if (!strcmp(target, "run") || !isFile(target))
		{
			// Parse arguments
			tokenize(command, &tokens);

			// Run the command
			runProcess(tokens[0], tokens);
		}
	}
}

/*
 * Count tokens in a sentence.
 */
static int countTokens(const char *sentence, int *longest)
{
	int i, count, lenght;
	lenght = *longest = i = count = 0;

	while (isEnd(sentence[i]) || isSpace(sentence[i])) i++;

	// Count the first (n - 1) tokens
	while (sentence[i])
	{
		if (isSpace(sentence[i]))
		{
			if (*longest < lenght)
				*longest = lenght;
			lenght = 0;
			count++;
			i++;
			while (isEnd(sentence[i]) || isSpace(sentence[i])) i++;
		}
		else
		{
			i++;
			lenght++;
		}
	}

	// Count the last token
	lenght--;
	if (*longest < lenght)
		*longest = lenght;

	return count + 1;
}

/*
 * Tokenize a string accordingly to a given delimiter.
 */
static int tokenize(char *sentence, char ***tokens)
{
	int k, i, j, longest, count;

	longest = 0;
	count = countTokens(sentence, &longest);

	// Allocate space for array of strings
	*tokens = (char **) malloc((count +1 ) * sizeof (char *));
	if (!*tokens) errorAndDie("malloc");

	// Allocate space for each string in the array and tokenize it
	i = 0;
	while (isEnd(sentence[i]) || isSpace(sentence[i])) i++;

	for (k = 0; k < count; k++)
	{
		(*tokens)[k] = (char *) malloc((longest + 1) * sizeof (char));

		j = 0;
		while (sentence[i] && !isEnd(sentence[i]) && !isSpace(sentence[i]))
			(*tokens)[k][j++] = sentence[i++];

		// Null-terminated string
		(*tokens)[k][j] = (char) 0;

		while (isEnd(sentence[i]) || isSpace(sentence[i])) i++;
	}

	(*tokens)[k] = NULL;

	return count;
}

static inline int isEnd(int c)
{
    return c == '\n' || c == EOF;
}

static inline int isSpace(int c)
{
    return c == ' ' || c == '\t';
}

/*
 * Run a specific process.
 * Wait until it terminates.
 * Input:	path, the path to the file to run
 *          args, the list of arguments
 */
static void runProcess(char *path, char *argv[])
{
	pid_t pid;

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child
	if (pid == 0)
	{
		execvp(path, argv);
		errorAndDie("execvp");
	}

	// Wait until the child terminates
	if (wait(NULL) < 0)
		errorAndDie("wait");
}

/*
 * Check if a file exists.
 */
static inline int isFile(char *path)
{
	return access(path, F_OK) != -1;
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
