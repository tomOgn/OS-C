/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.05.29.pdf

@author: Tommaso Ognibene
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

/*
 * Comparison function for iNode
 * Input:
 * 			2 dirent pointers
 * Output:
 * 			1,	if iNode[a] >  iNode[b]
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
 * Scan a directory ->
 * Order entries by iNode number ->
 * Print name and iNode of each file.
 */
void scanDirectory(char *dir)
{
	int result, iterator;
	struct dirent **files;

	result = scandir(dir, &files, NULL, iNodeComparison);

	// Check if any errors occurred
	if (result < 0)
	{
		perror("scandir()");
		exit(EXIT_FAILURE);
	}

	// Loop through directory entries
	for(iterator = 0; iterator < result; iterator++)
	{
		// Do the printing stuff
		printf("%s/%s %li\n", dir, files[iterator]->d_name, (long)files[iterator]->d_ino);

		// Garbage collection
		free(files[iterator]);
	}
	free(files);
}

/*
 * Scan a directory ->
 * Order entries by iNode ->
 * Group hard links and print them.
 */
void printHardLinks(char *dir)
{
	int result, i;
	struct dirent **files;

	result = scandir(dir, &files, NULL, iNodeComparison);

	// Check if any errors occurred
	if (result < 0)
	{
		perror("scandir()");
		exit(EXIT_FAILURE);
	}

	// Check if directory is empty
	if (!result) return;

	// Print first element
	printf("%s/%s ", dir, files[0]->d_name);

	// Loop through directory entries
	for(i = 1; i < result; i++)
	{
		// If (iNode[i] != iNode[i - 1]) => not hard links => break line
		if ((long)files[i]->d_ino != (long)files[i - 1]->d_ino) printf("\n");

		// Print file name
		printf("%s/%s ", dir, files[i]->d_name);
	}
}

// Exercise 1
int run1(int argc, char *argv[])
{
    char cwd[1024];

    // If (number of parameters ...
    switch(argc)
    {
    	// ... == 0) => Use the current directory
    	case 1:
    		if (!getcwd(cwd, sizeof(cwd)))
    		{
    			perror("getcwd()");
    			exit(EXIT_FAILURE);
    		}
    		scanDirectory(cwd);
    		exit(EXIT_SUCCESS);
    	// ... == 1) => Use the given directory
    	case 2:
    		scanDirectory(argv[1]);
    		exit(EXIT_SUCCESS);
    	// ... > 1) => Wrong input
    	default:
    		printf("The function requires 0 or 1 parameters.\n");
    		exit(EXIT_FAILURE);
    }
}

// Exercise 2
int run2(int argc, char *argv[])
{
    char cwd[1024];

    // If (number of parameters ...
    switch(argc)
    {
    	// ... == 0) => Use the current directory
    	case 1:
    		if (!getcwd(cwd, sizeof(cwd)))
    		{
    			perror("getcwd()");
    			exit(EXIT_FAILURE);
    		}
    		printHardLinks(cwd);
    		exit(EXIT_SUCCESS);
    	// ... == 1) => Use the given directory
    	case 2:
    		printHardLinks(argv[1]);
    		exit(EXIT_SUCCESS);
    	// ... > 1) => Wrong input
    	default:
    		printf("The function requires 0 or 1 parameters.\n");
    		exit(EXIT_FAILURE);
    }
}
