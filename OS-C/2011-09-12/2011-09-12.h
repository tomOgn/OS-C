/*
Prova Pratica di Laboratorio di Sistemi Operativi
12 settembre 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.09.12.pdf

@author: Tommaso Ognibene
*/

// Pre-processor directives
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Get the full path of a file.
 * Input:
 * 			dirPath, 	the directory path
 * 			fileName, 	the file name
 * Output:
 * 			pointer to the file path
 */
static inline char *getFilePath(char *dirPath, char *fileName)
{
	int lenght = strlen(dirPath) + strlen(fileName) + 2;
	char *filePath = (char *) malloc(lenght * sizeof(char));

	if (!filePath)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	sprintf(filePath, "%s/%s", dirPath, fileName);
	filePath[lenght - 1] = (char)0;

	return filePath;
}

/*
 * Check, byte by byte, if two files have same content.
 * Input:
 * 			f1, pointer to the file 1
 * 			f2, pointer to the file 2
 * Output:
 * 			TRUE,	if the files have same content
 * 			FALSE, 	else
 */
static inline int sameContent(FILE *f1, FILE *f2)
{
	while (!feof(f1) && fgetc(f1) == fgetc(f2));
	return feof(f1) && feof(f2)? TRUE : FALSE;
}

/*
 * Check if a file name has a particular extension.
 * Input:
 * 			fileName, 		the file name
 * 			extensions.		the array of extensions
 * 			n,				dimension of the array
 *
 * Output:
 * 			1, if the entry has one of the possible extensions
 * 			0, else
 */
int hasExtension(const char *fileName, const char **extensions, const int n)
{
	char *dot = strrchr(fileName, '.');
	int output = FALSE;
	int i = 0;

	if (dot)
	{
		while (i < n && !output)
		{
			if (!strcmp(dot + sizeof(char), extensions[i]))
			{
				output = TRUE;
			}
			else i++;
		}
	}

	return output;
}

/*
 * Filter function for Directories
 * Input:
 * 			entry, pointer to the directory entry
 * Output:
 * 			1, if the entry has .h or .c extension
 * 			0, else
 */
int filterDirHandC(const struct dirent *entry)
{
	const char *extensions[] = { "c", "h"};
	return entry->d_type == DT_REG && hasExtension(entry->d_name, extensions, 2);
}

/*
 * Scan two directories. Order the entries alphabetically.
 * Print if a file is in dirA but not in dirB and vice versa.
 * Input:	dirA,	path to directory A
 * 			dirB,	path to directory B
 * Output:	void
 */
void compareDirectories(char *dirA, char *dirB)
{
	int result, countA, countB, itA, itB;
	struct dirent **filesA, **filesB;
	char *pathA, *pathB;
	FILE *fileA, *fileB;
	struct stat infoA, infoB;

	countA = scandir(dirA, &filesA, filterDirHandC, alphasort);
	countB = scandir(dirB, &filesB, filterDirHandC, alphasort);

	// Check if any errors occurred
	if (countA < 0 || countB < 0)
	{
		perror("scandir");
		exit(EXIT_FAILURE);
	}

	// Loop through directory entries
	itA = itB = 0;
	while (itA < countA && itB < countB)
	{
		result = strcmp(filesA[itA]->d_name, filesB[itB]->d_name);
		if (result > 0)
		{
			printf("%s not in %s\n", filesB[itB]->d_name, dirA);
			itB++;
		}
		else if (result < 0)
		{
			printf("%s not in %s\n", filesA[itA]->d_name, dirB);
			itA++;
		}
		else
		{
			pathA = getFilePath(dirA, filesA[itA]->d_name);
			pathB = getFilePath(dirB, filesB[itB]->d_name);
			fileA = fopen(pathA, "r");
			fileB = fopen(pathB, "r");
			if (!fileA || !fileB)
			{
				perror("fopen");
				exit(EXIT_FAILURE);
			}
			stat(pathA, &infoA);
			stat(pathB, &infoB);
			if (infoA.st_size == infoB.st_size)
			{
				if (!sameContent(fileA, fileB))
				{
					printf("%s %s differ\n", pathA, pathB);
				}
			}
			else
			{
				printf("%s %s differ\n", pathA, pathB);
			}
			itA++;
			itB++;
		}
	}
	if (itA < countA)
	{
		for (; itA < countA; itA++)
		{
			printf("%s not in %s\n", filesA[itA]->d_name, dirB);
		}
	}
	else
	{
		for (; itB < countB; itB++)
		{
			printf("%s not in %s\n", filesB[itB]->d_name, dirA);
		}
	}
}

extern void run(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("The function requires two parameters to be passed in.\n");
		exit(EXIT_FAILURE);
	}

	compareDirectories(argv[1], argv[2]);

	exit(EXIT_SUCCESS);
}
