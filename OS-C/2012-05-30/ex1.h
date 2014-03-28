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

// Constants
#define Delimiter ":"
#define MiniMakeFile "minimakefile"

// Function declarations
static inline void parseLine(char *line, char *delimiter);
static void runProcess(char *path, char *argv[]);
static inline void tokenize(char *sentence, int count, char **tokens[]);
static inline int countTokens(char *sentence);
static inline int isFile(char *path);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	FILE *fp;
	char *line = NULL;
	ssize_t read;
	char path[1024];
    size_t len = 0;

	// Open the minimakefile
	strcpy(path, "./");
	strcat(path, MiniMakeFile);
	printf("path = %s\n", path);

	fp = fopen(path, "r");
	if (!fp)
		printAndDie("Mini Make File not found.");

	// Read the file line by line
	while ((read = getline(&line, &len, fp)) != -1)
		parseLine(line, ":");
}

/*
 * Parse a line of text and extract the commands to run.
 * Input:  line,      line of text
 *         delimiter, delimiter between target and command
 */
static inline void parseLine(char *line, char *delimiter)
{
	char tmp[strlen(line)];
	char *path;
	char *target, *command;
	char **tokens;
	int count;
	int i;
	printf("line = %s\n", line);
	// Extract target and command
	strcpy(tmp, line);
	target = strtok(tmp, delimiter);
	command = strtok(NULL, delimiter);
	printf("target = %s\n", target);
	printf("command = %s\n", command);

	// If the target does not exist
	if (!isFile(target))
	{
		// Parse arguments
		count = countTokens(command);
		printf("count = %d\n", count);
		tokens = (char **) malloc(sizeof (char *) * (count + 1));

//		tokenize(command, count, &tokens);
//		printf("token 1 = %s\n", tokens[0]);
//		printf("token 2 = %s\n", tokens[1]);


		tokens[0] = strtok(command, " ");
		for (i = 1; i < count; i++)
			tokens[i] = strtok(NULL, " ");
		tokens[count] = NULL;

		printf("token 1 = %s\n", tokens[0]);
		printf("token 2 = %s\n", tokens[1]);

		path = tokens[0];
		printf("path = %s\n", path);
		// Run the command
		runProcess(path, tokens);
	}
}

static inline int isEnd(int c)
{
    return c == '\n' || c == EOF;
}

static inline int isSpace(int c)
{
    return c == ' ' || c == '\t';
}

static inline int getLine(char *line, char *delimiter, FILE *fp, char *buffer, size_t buflen)
{
    char *end = buffer + buflen - 1; /* Allow space for null terminator */
    char *dst = buffer;
    int c;

	char tmp[strlen(line)];
	char *path;
	char *target, *command;
	char **tokens;
	int count;
	int i;
	printf("line = %s\n", line);

	// Extract target and command
	strcpy(tmp, line);
	target = strtok(tmp, delimiter);
	command = strtok(NULL, delimiter);
	printf("target = %s\n", target);
	printf("command = %s\n", command);

	// If the target does not exist
	if (!isFile(target))
	{
		// Parse arguments
		count = countTokens(command);
		printf("count = %d\n", count);
		tokens = (char **) malloc(sizeof (char *) * (count + 1));

		tokens[0] = strtok(command, " ");

		for (i = 1; i < count; i++)
		{
			int lenght = lengthWord(char *line, int start);

		}
			tokens[i] = strtok(NULL, " ");
		tokens[count] = NULL;

		printf("token 1 = %s\n", tokens[0]);
		printf("token 2 = %s\n", tokens[1]);

		path = tokens[0];
		printf("path = %s\n", path);
		// Run the command
		runProcess(path, tokens);


    c = getc(fp);
    while (!isEnd(c) && dst < end)
    {
    	if (!isSpace(c))
    		*dst++ = c;
    	c = getc(fp);
    }
    *dst = '\0';
    return((c == EOF && dst == buffer) ? EOF : dst - buffer);
}

/*
 * Get the length of a word.
 * Input:  line,  line of text
 *         start, pointer to the first character of the word
 * Output: length of the word
 */
static inline int lengthWord(char *line, int start)
{
	int end = start;

	while (!isEnd(line[end]) && !isSpace(line[end]))
		end++;

	return end - start;
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

	// Child process
	if (pid == 0)
	{
		if (execvp(path, argv) < 0)
			errorAndDie("execvp");

		exit(EXIT_SUCCESS);
	}

	// Wait until the child process terminates
	if (wait(NULL) < 0)
		errorAndDie("wait");
}

/*
 * Tokenize a sentence.
 * Input:  sentence, the sentence
 * Output: char **, array of tokens
 */
static inline void tokenize(char *sentence, int count, char **tokens[])
{
	char tmp[strlen(sentence)];
	int i;

	printf("sentence = %s\n", sentence);
	strcpy(tmp, sentence);
	printf("tmp = %s\n", tmp);

	(*tokens)[0] = strtok(tmp, " ");
	for (i = 1; i < count; i++)
		(*tokens)[i] = strtok(NULL, " ");
	(*tokens)[count] = NULL;

	printf("token 1 = %s\n", (*tokens)[0]);
	printf("token 2 = %s\n", (*tokens)[1]);
}

/*
 * Count tokens in a sentence.
 * Input:  sentence, the sentence
 * Output: int, number of tokens
 */
static inline int countTokens(char *sentence)
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

/*
 * Check if a path leads to an existing file.
 * Input:	path,  the file path
 * Output:	true,  if the file exists
 *          false, otherwise
 */
static inline int isFile(char *path)
{
	FILE *fp = fopen(path, "r");

	if (fp)
	{
		fclose(fp);
		return TRUE;
	}

	return FALSE;
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
