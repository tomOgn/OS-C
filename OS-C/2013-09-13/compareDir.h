/*
Prova Pratica di Laboratorio di Sistemi Operativi
13 settembre 2013
Esercizio 1

URL = http://www.cs.unibo.it/~renzo/so/pratiche/2013.09.13.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <fcntl.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Function declarations
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
static inline void compareMD5(char **files, int startOffset, int endOffset);
static inline unsigned char *computeMD5(char *fileName);
int compareBySize(const struct dirent **file1, const struct dirent **file2);
static void scanDirectory(char *dir);
static inline char *getAbsolutePath(char *dirPath, char *fileName);

// Data Types
typedef struct
{
	char *Path;
	unsigned char *Digest;
} MD5File;

extern void run(int argc, char *argv[])
{
	if (argc != 2)
		printAndDie("The function requires one parameter to be passed in.");

	scanDirectory(argv[1]);
}

/*
 * Scan a directory. Order entries by iNode number.
 * Print name and iNode of each file.
 * Input: dir, the directory
 */
static void scanDirectory(char *dir)
{
	int count, i, startOffset;
	struct dirent **fileName;
	char **filePath;
	struct stat buffer;
	int previousSize;
	
	printf("dir = %s\n", dir);
 	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		fprintf(stdout, "Current working dir: %s\n", cwd);
	else
		perror("getcwd() error");
	char *folder = getAbsolutePath(cwd, dir);
	printf("folder = %s\n", folder);
	count = scandir(folder, &fileName, NULL, compareBySize);
	printf("count = %d\n", count);
	if (count < 0)
		errorAndDie("scandir");

	filePath = (char **) malloc(count * sizeof (char *));
	for (i = 0; i < count; i++)
		filePath[i] = getAbsolutePath(dir, fileName[i]->d_name);
		
	if (stat(filePath[0], &buffer) < 0)
		errorAndDie("stat");

	startOffset = 0;
	previousSize = buffer.st_size;

	// Loop through directory entries
	for (i = 1; i < count; i++)
	{
		if (stat(filePath[i], &buffer) < 0)
			errorAndDie("stat");

		if (buffer.st_size != previousSize)
		{
			compareMD5(filePath, startOffset, i - 1);
			startOffset = i;
			previousSize = buffer.st_size;
		}
	}
	
	if (startOffset < count - 1)
	{
		printf("Last ones\n");
	}
}

/*
 * Comparison function.
 * Compare the size of two files.
*/
int compareBySize(const struct dirent **file1, const struct dirent **file2)
{
	struct stat buf1, buf2;

	if (stat((*file1)->d_name, &buf1) < 0 || stat((*file2)->d_name, &buf2) < 0)
		errorAndDie("stat");

	return (buf1.st_size > buf2.st_size) - (buf1.st_size < buf2.st_size);
}

static inline void compareMD5(char **files, int startOffset, int endOffset)
{
	int i;
	MD5File md5file[endOffset - startOffset + 1];

	for (i = startOffset; i <= endOffset; i++)
	{
		md5file[i - startOffset].Digest = computeMD5(files[i]);
		md5file[i - startOffset].Path = files[i];
	}
	for (i = 0; i <= endOffset - startOffset; i++)
	{
		printf("%s\n", md5file[i].Path);
	}	
}

/*
 * Compute the MD5 hash of a file.
 */
static inline unsigned char *computeMD5(char *fileName)
{
    unsigned char chunk[1024];
    int bytes;
    FILE *inFile;
    MD5_CTX mdContext;

    unsigned char *digest = (unsigned char *) malloc(sizeof (unsigned char) * 128);

    if (!(inFile = fopen (fileName, "rb")))
    	printAndDie("The file can't be opened.");

    MD5_Init(&mdContext);
    while ((bytes = fread(chunk, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, chunk, bytes);
    MD5_Final(digest, &mdContext);

    fclose(inFile);
    
    return digest;
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
