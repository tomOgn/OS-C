/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.06.21.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

// Function declarations
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
int iNodeComparison(const struct dirent **a, const struct dirent **b);
void scanDirectory(char *dir);

/*
 * Comparison function for iNode
 * Input:   a & b,  pointers to directory entries
 * Output:  1,	    if iNode[a] >  iNode[b]
 * 			0,	    if iNode[a] == iNode[b]
 * 		   -1,      if iNode[a] <  iNode[b]
 */
int iNodeComparison(const struct dirent **a, const struct dirent **b)
{
    long iNodeA, iNodeB;

    iNodeA = (long)(*a)->d_ino;
    iNodeB = (long)(*b)->d_ino;

    return (iNodeA > iNodeB) - (iNodeA < iNodeB);
}

/*
 * Scan a directory. Order entries by iNode number.
 * Print name and iNode of each file.
 * Input: dir, the directory
 */
void scanDirectory(char *dir)
{
	int result, it;
	struct dirent **files;

	result = scandir(dir, &files, NULL, iNodeComparison);

	if (result < 0)
		errorAndDie("scandir");

	// Loop through directory entries
	for(it = 0; it < result; it++)
	{
		// Do the printing stuff
		printf("%s/%s %li\n", dir, files[it]->d_name, (long)files[it]->d_ino);

		// Garbage collection
		free(files[it]);
	}
	free(files);
}

static void run(int argc, char *argv[])
{
    char *cwd = ".";

    // If (number of parameters ...
    switch(argc)
    {
    	// ... == 0) => Use the current directory
    	case 1:
    		scanDirectory(cwd);
    		exit(EXIT_SUCCESS);
    	// ... == 1) => Use the given directory
    	case 2:
    		scanDirectory(argv[1]);
    		exit(EXIT_SUCCESS);
    	// ... > 1) => Wrong input
    	default:
    		printAndDie("The function requires 0 or 1 parameters.\n");
    }
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
