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
static inline void errorAndDie(const char *msg);
static void RedirectIOAndRunProcess(const char *outFile, const char *inFile, const char *path, char *command[]);
static void parseArguments(int argc, char **argv);

static void RedirectIOAndRunProcess(const char *outFile, const char *inFile, const char *path, char *command[])
{
	int pid, outFD, inFD;

	outFD = inFD = 0;
	if (inFile)
	{
		inFD = open(outFile, O_RDONLY);
		if (inFD < 0)
			errorAndDie("open");
	}
	if (outFile)
	{
		outFD = open(outFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		if (outFD < 0)
			errorAndDie("open");
	}

	switch (pid = fork())
	{
	case -1:
		errorAndDie("fork");
		break;
	case 0:
		if (inFile)
		{
			if (dup2(inFD, 0) < 0)
				errorAndDie("dup2");
			close(inFD);
		}
		if (outFile)
		{
			if (dup2(outFD, 1) < 0)
				errorAndDie("dup2");
			close(outFD);
		}
	    if (execvp(path, command) == -1)
	    	errorAndDie("execvp");
	    break;
	default:
		if (inFile)
			close(inFD);
		if (outFile)
			close(outFD);
	}
}

static void parseArguments(int argc, char **argv)
{
	int i, result, optIndex;
	char **command;
	const char *shortOptions = "o:i:h";
	char *fileName, *outFile, *inFile;
	extern int opterr;

	static struct option longOptions[] =
	{
		{"in",		required_argument, 0,  'i' },
		{"out",		required_argument, 0,  'o' },
		{"help",	no_argument, 	   0,  'h' },
		{0,         0,                 0,   0  }
	};

	opterr = optIndex = 0;
	result = TRUE;
	outFile = inFile = NULL;
	result = getopt_long(argc, argv, shortOptions, longOptions, &i);

	while (result != -1 && result != '?')
	{
		switch (result)
		{
		case 'i':
			inFile = optarg;
			optIndex = optind;
			break;
		case 'o':
			outFile = optarg;
			optIndex = optind;
			break;
		case 'h':
			printf("Usage: [options] command\n");
			printf(" options:\n");
			printf("  -i --in    required_argument   Redirect input.\n");
			printf("  -o --out   required_argument   Redirect output.\n");
			printf("  -h --help  no_argument         Show this help.\n");
			exit(EXIT_SUCCESS);
		}

		i = 0;
		result = getopt_long(argc, argv, shortOptions, longOptions, &i);
	}

	if (optIndex == argc)
	{
		printf("Wrong input. Run 'redir -h' for help.\n");
		exit(EXIT_FAILURE);
	}

	command = (char **) malloc((argc - optIndex + 1) * sizeof(char **));
	fileName = argv[optIndex];

	i = 0;
	while (optIndex < argc)
		command[i++] = argv[optIndex++];
	command[i] = NULL;

	RedirectIOAndRunProcess(outFile, inFile, fileName, command);

	printf("Done!\n");
	exit(EXIT_SUCCESS);
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

extern void run(int argc, char *argv[])
{
	parseArguments(argc, argv);
}
