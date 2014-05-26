/*
Prova Pratica di Laboratorio di Sistemi Operativi
19 gennaio 2011
Esercizio 1 & 2

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.01.19.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

// Function declarations
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
static void redirectAndRun(char *outFile, char *inFile, char *path, char *command[]);
static void parseArguments(int argc, char **argv);

// Entry point
extern void run(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

/*
 * Parse arguments accordingly to a given set of options.
 * Input: argc, the argument counter
 *        argv, the argument vector
 */
static void parseArguments(int argc, char **argv)
{
	int i, j, result;
	char **command;
	char *fileName, *outFile, *inFile;

	// Short-named options
	const char *shortOptions = "o:i:h";
	// Long-named options
	static struct option longOptions[] =
	{
		{"in",		required_argument, 0,  'i' },
		{"out",		required_argument, 0,  'o' },
		{"help",	no_argument, 	   0,  'h' },
		{0,         0,                 0,   0  }
	};

	result = True;
	outFile = inFile = NULL;

	// Disable default error message
	opterr = 0;

	// Parse options
	result = getopt_long(argc, argv, shortOptions, longOptions, NULL);
	if (result < 0)
		printAndDie("Wrong input. Run 'redir -h' for help.");

	j = 1;
	while (result != -1 && result != '?')
	{
		// Get the option value
		switch (result)
		{
		case 'i':
			inFile = optarg;
			j = optind;
			break;
		case 'o':
			outFile = optarg;
			j = optind;
			break;
		case 'h':
			printf("Usage: [options] command\n");
			printf(" options:\n");
			printf("  -i --in    required_argument   Redirect input.\n");
			printf("  -o --out   required_argument   Redirect output.\n");
			printf("  -h --help  no_argument         Show this help.\n");
			exit(EXIT_SUCCESS);
		}
		result = getopt_long(argc, argv, shortOptions, longOptions, NULL);
	}

	// Parse command
	command = (char **) malloc((argc - optind + 1) * sizeof (char *));
	fileName = argv[j];
	i = 0;
	while (j < argc)
		command[i++] = argv[j++];
	command[i] = NULL;

	redirectAndRun(outFile, inFile, fileName, command);
}

/*
 * Re-direct [input, output] and run a command
 * Input: outFile, output file
 *        inFile,  input file
 *        command, command
 *        args,    command arguments
 */
static void redirectAndRun(char *outFile, char *inFile, char *command, char *args[])
{
	int pid, outFD, inFD;

	outFD = inFD = 0;
	if (inFile)
	{
		inFD = open(inFile, O_RDONLY);
		if (inFD < 0)
			errorAndDie("open");
	}
	if (outFile)
	{
		outFD = open(outFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		if (outFD < 0)
			errorAndDie("open");
	}

	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		if (inFile)
		{
			if (dup2(inFD, STDIN_FILENO) < 0)
				errorAndDie("dup2");

			if (close(inFD) < 0)
				 errorAndDie("close");
		}
		if (outFile)
		{
			if (dup2(outFD, STDOUT_FILENO) < 0)
				errorAndDie("dup2");
			if (close(outFD) < 0)
				 errorAndDie("close");
		}
	    execvp(command, args);
	    errorAndDie("execvp");
	}

	// Parent process
	if (inFile)
		if (close(inFD) < 0)
			 errorAndDie("close");
	if (outFile)
		if (close(outFD) < 0)
			 errorAndDie("close");
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
