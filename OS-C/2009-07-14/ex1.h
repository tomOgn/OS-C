/*
Prova Pratica di Laboratorio di Sistemi Operativi
14 luglio 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.09.23.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants
#define _GNU_SOURCE

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

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

static void redirectInputInBuffer(void)
{
	FILE *in;
	int result;
	size_t size;
	char *ptr;


	in = open_memstream(&ptr, &size);
	if (!in)
		errorAndDie("open_memstream");

	result = dup2(fileno(in), STDIN_FILENO);
	if (result < 0)
		errorAndDie("dup2");
	fclose(in);

	printf("ptr=%s\n", ptr);
	free(ptr);
}

extern void run(int argc, char *argv[])
{
	redirectInputInBuffer();
}
