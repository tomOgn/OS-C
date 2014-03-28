/*
Prova Pratica di Laboratorio di Sistemi Operativi
13 settembre 2013
Esercizio 1

http://www.cs.unibo.it/~renzo/so/pratiche/2013.09.13.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function declarations
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
static inline void computeMD5(char *fileName, unsigned char **digest);

// Constants
extern void run(int argc, char *argv[])
{
//	if (argc != 2)
//		printAndDie("The function requires one parameter to be passed in.");

	unsigned char *digest;
	computeMD5("fff", &digest);
}

static inline void computeMD5(char *fileName, unsigned char **digest)
{
    unsigned char chunk[1024];
    int bytes;
    FILE *inFile;
    MD5_CTX mdContext;

    (*digest) = (unsigned char *) malloc(sizeof (unsigned char) * 128);

    inFile = fopen (fileName, "rb");
    if (!inFile)
    	printAndDie("The file can't be opened.");

    MD5_Init(&mdContext);
    while ((bytes = fread(chunk, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, chunk, bytes);
    MD5_Final((*digest), &mdContext);

    fclose (inFile);
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
