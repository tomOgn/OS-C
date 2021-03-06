/*
Prova Pratica di Laboratorio di Sistemi Operativi
12 settembre 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.09.12.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>

// Function declarations
static inline int isDirectory(char *path);
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
static inline char *getAbsolutePath(char *dirPath, char *fileName);
static inline int sameContent(FILE *f1, FILE *f2);
static int hasExtension(const char *fileName, const char **extensions, int n);
int filterExtensions(const struct dirent *entry);
static void compareDirectories(char *dirA, char *dirB);

extern void run(int argc, char *argv[])
{
	if (argc != 3)
		printAndDie("The function requires two parameters to be passed in.");

	if (!isDirectory(argv[1]) || !isDirectory(argv[2]))
		printAndDie("The parameters should be two existing directories.");

	compareDirectories(argv[1], argv[2]);
}

/*
 * Scan two directories. Order the entries alphabetically.
 * Print if a file is in dirA but not in dirB and vice versa.
 * Input: dirA & dirB, path to directories A and B
 */
static void compareDirectories(char *dirA, char *dirB)
{
	int result, countA, countB, itA, itB;
	struct dirent **filesA, **filesB;

	countA = scandir(dirA, &filesA, filterExtensions, alphasort);
	countB = scandir(dirB, &filesB, filterExtensions, alphasort);
	if (countA < 0 || countB < 0)
		errorAndDie("scandir");

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
			itA++;
			itB++;
		}
	}
	if (itA < countA)
		for (; itA < countA; itA++)
			printf("%s not in %s\n", filesA[itA]->d_name, dirB);
	else
		for (; itB < countB; itB++)
			printf("%s not in %s\n", filesB[itB]->d_name, dirA);
}

/*
 * Check if a file name has a particular extension.
 * Input:   fileName, 	file name
 * 			extensions,	array of possible extensions
 * 			n,			dimension of the array
 * Output:
 * 			1,          if the entry has one of the possible extensions
 * 			0,          else
 */
static inline int hasExtension(const char *fileName, const char **extensions, int n)
{
	char *extension;
	int i;

	if (!(extension = strrchr(fileName, '.')))
		return False;

	extension += sizeof (char);
	for (i = 0; i < n; i++)
		if (!strcmp(extension, extensions[i]))
			return True;

	return False;
}

/*
 * Filter function for directories
 * Input:  entry, pointer to the directory entry
 * Output: 1,     if the entry has .h or .c extension
 * 		   0,     else
 */
int filterExtensions(const struct dirent *entry)
{
	const char *exts[] = { "c", "h" };

	return entry->d_type == DT_REG && hasExtension(entry->d_name, exts, 2);
}

/*
 * Check if a file is a directory
 * Input:  path, file path
 * Output: 1,    if the file is a directory
 * 		   0,    else
 */
static inline int isDirectory(char *path)
{
	struct stat info;

	if (stat(path, &info) < 0)
		errorAndDie("stat");

	return info.st_mode & S_IFDIR;
}

/*
 * Get the full path of a file.
 * Input:  dirPath,  directory path
 * 		   fileName, file name
 * Output: pointer to the file path
 */
static inline char *getAbsolutePath(char *dirPath, char *fileName)
{
	int lenght;
	char *filePath;

	lenght = strlen(dirPath) + strlen(fileName) + 1;
	filePath = (char *) malloc(lenght * sizeof (char));
	if (!filePath)
		errorAndDie("malloc");

	sprintf(filePath, "%s/%s", dirPath, fileName);

	return filePath;
}

/*
 * Check, byte by byte, if two files have same content.
 * Input:   f1,    file 1
 * 		    f2,    file 2
 * Output:  1,     if they have the same content
 * 			0,     else
 */
static inline int sameContent(FILE *f1, FILE *f2)
{
	while (!feof(f1) && fgetc(f1) == fgetc(f2));

	return feof(f1) && feof(f2);
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
