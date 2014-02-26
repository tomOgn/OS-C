/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2013
Esercizio 2

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
static void printHardLinks(char *dir);

/*
 * Comparison function for iNode
 * Input:   a,  pointer to a directory entry
 *          b,  pointer to a directory entry
 * Output:  1,	if iNode[a] >  iNode[b]
 * 			0,	if iNode[a] == iNode[b]
 * 		   -1,  if iNode[a] <  iNode[b]
 */
int iNodeComparison(const struct dirent **a, const struct dirent **b)
{
    long iNodeA = (long)(*a)->d_ino;
    long iNodeB = (long)(*b)->d_ino;
    return (iNodeA > iNodeB) - (iNodeA < iNodeB);
}

/*
 * Scan a directory. Order entries by iNode.
 * Group hard links and print them.
 * Input: dir, the directory
 */
static void printHardLinks(char *dir)
{
	int result, i;
	struct dirent **files;

	result = scandir(dir, &files, NULL, iNodeComparison);
	if (result < 0)
		errorAndDie("scandir");

	// Check if directory is empty
	if (!result) return;

	// Print first element
	printf("%s/%s ", dir, files[0]->d_name);

	// Loop through directory entries
	for(i = 1; i < result; i++)
	{
		// If (iNode[i] != iNode[i - 1]) => not hard links => break line
		if ((long)files[i]->d_ino != (long)files[i - 1]->d_ino)
			printf("\n");

		// Print file name
		printf("%s/%s ", dir, files[i]->d_name);
	}
	printf("\n");
}

extern void run(int argc, char *argv[])
{
    char *cwd = ".";

    // If (number of parameters ...
    switch(argc)
    {
    	// ... == 0) => Use the current directory
    	case 1:
    		printHardLinks(cwd);
    		break;
    	// ... == 1) => Use the given directory
    	case 2:
    		printHardLinks(argv[1]);
    		break;
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
