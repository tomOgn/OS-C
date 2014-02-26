// FUN-ctions Repository
#ifndef FUNREPO_H
#define FUNREPO_H

// Dependencies
#include "../const.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Constants
// Max number of events to process at one go
#define MAX_EVENTS 1024
// Max lenght of the filename
#define LEN_NAME 16
// Size of one event
#define EVENT_SIZE (sizeof (struct inotify_event))
// Buffer to store the events
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

// Function declarations
static inline int isNatural(int n);
static inline int isInteger(char *n);
static inline int isSignalNumber(int n);
static void compareDirectories(char *dirA, char *dirB);
int filterExtensions(const struct dirent *entry);
static int hasExtension(const char *fileName, const char **extensions, int n);
static inline int sameContent(FILE *f1, FILE *f2);
static inline char *getAbsolutePath(char *dirPath, char *filePath);
int iNodeComparison(const struct dirent **a, const struct dirent **b);
static void scanDirectory(char *dir);
static void spyDirectory(char *directory);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static int getLenghtOfLine(char *path, int theLine, int *lenght);
static inline int isDirectory(char *path);
static inline int isTextFile(char *path);
static inline int isHiddenFile(char *path);
static inline int isRegularFile(char *path);

/*
 * Check if a number is natural.
 * Input:  n, number
 * Output: 1, if the number is natural
 * 		   0, else
 */
static inline int isNatural(int n)
{
	return n >= 0 && n == floor(n);
}

/*
 * Check if a string represents an integer.
 * Input:  n, string
 * Output: 1, if the string represents an integer
 * 		   0, else
 */
static inline int isInteger(char *n)
{
	long int result = strtol(n, NULL, 10);

	return result != LONG_MIN && result != LONG_MAX;
}

/*
 * Check if a number is within the range of supported real-time signals.
 * Input:  n, number
 * Output: 1, if the number is within the range
 * 		   0, else
 */
static inline int isSignalNumber(int n)
{
	return isNatural(n) && n >= 0 && n <= 64;
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
 * Filter function for directories
 * Input:  entry, pointer to the directory entry
 * Output: 1,     if the entry has .h or .c extension
 * 		   0,     else
 */
int filterExtensions(const struct dirent *entry)
{
	const char *exts[] = { "c", "h"};

	return entry->d_type == DT_REG && hasExtension(entry->d_name, exts, 2);
}

/*
 * Check if a file name has a particular extension.
 * Input:   fileName, 	file name
 * 			extensions,	array of possible extensions
 * 			n,			dimension of the array
 *
 * Output:
 * 			1,          if the entry has one of the possible extensions
 * 			0,          else
 */
static int hasExtension(const char *fileName, const char **extensions, int n)
{
	char *extension = strrchr(fileName, '.') + 1;
	int output = FALSE;
	int i = 0;

	if (extension)
		while (i < n && !output)
		{
			if (!strcmp(extension, extensions[i]))
				output = TRUE;
			else
				i++;
		}

	return output;
}

/*
 * Check, byte by byte, if two files have same content.
 * Input:   f1,    pointer to file 1
 * 		    f2,    pointer to file 2
 * Output:  TRUE,  if they have the same content
 * 			FALSE, else
 */
static inline int sameContent(FILE *f1, FILE *f2)
{
	while (!feof(f1) && fgetc(f1) == fgetc(f2));

	return feof(f1) && feof(f2);
}

/*
 * Get the full path of a file.
 * Input:  dirPath,  directory path
 * 		   fileName, file name
 * Output: pointer to the file path
 */
static inline char *getAbsolutePath(char *dirPath, char *fileName)
{
	int lenght = strlen(dirPath) + strlen(fileName) + 1;
	char *filePath = (char *) malloc(lenght * sizeof(char *));
	if (!filePath)
		errorAndDie("malloc");

	sprintf(filePath, "%s/%s", dirPath, fileName);

	return filePath;
}

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
	for (it = 0; it < result; it++)
	{
		// Do the printing stuff
		printf("%s/%s %li\n", dir, files[it]->d_name, (long)files[it]->d_ino);

		// Garbage collection
		free(files[it]);
	}
	free(files);
}

static void spyDirectory(char *directory)
{
	int fd, wd, length, i;
	char buffer[BUF_LEN];
	char **path = (char **) malloc(sizeof(char **) * 2);
	struct inotify_event *event;

	path[0] = directory;
	i = 0;

	// Create an inotify instance
	fd = inotify_init();

	if (fd < 0)
		errorAndDie("inotify_init");

	// Create a watch list
	wd = inotify_add_watch(fd, directory, IN_CREATE);
	if (wd < 0)
		errorAndDie("inotify_add_watch");

	printf("Watching %s\n", directory);

	while (TRUE)
	{
		i = 0;
		length = read(fd, buffer, BUF_LEN);

		if (length < 0)
			errorAndDie("read");

		while (i < length)
		{
			event = (struct inotify_event *) &buffer[i];
			if (event->len)
			{
				if (event->mask & IN_CREATE)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s was created.\n", event->name);
					else
						printf("The file %s was created.\n", event->name);
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}

	// Clean up
	inotify_rm_watch(fd, wd);
	close(fd);
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

/*
 * Find a specific line in a text file and get its lenght (i.e. number of characters).
 * Inpstatic inline int isDirectory(char *path);ut:
 * 			path,       file path
 * 			theLine,    line number
 * Output:
 * 			lenght,     lenght of the line
 *
 * 			1,          in case of success
 * 			0,          else
 */
static int getLenghtOfLine(char *path, int theLine, int *lenght)
{
	int c, count, line, output;
	FILE *stream;

	count = 0;
	line = 0;
	stream = fopen(path, "r");
	if (!stream)
		errorAndDie("fopen");

	while ((c = fgetc(stream)) != EOF && line < theLine)
	{
		if (c == '\n')
		{
			line++;
			if (line < theLine)
				count = 0;
		}
		else
			count++;
	}

	fclose(stream);

	if (line == theLine)
	{
		*lenght = count;
		output = TRUE;
	}
	else
	{
		*lenght = 0;
		output = FALSE;
	}

	return output;
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
 * Check if a file is textual
 * Input:  path, file pointer
 * Output: 1,    if the file is a text file
 * 		   0,    else
 */
static inline int isTextFile(char *path)
{
	int c;
	FILE *fp = fopen(path, "r");

	if (!fp)
		errorAndDie("fopen");

	while ((c = fgetc(fp)) != EOF && c >= 1 && c <= 127);
	fclose(fp);

	return c == EOF;
}

/*
 * Check if a file is hidden
 * Input:  path, file path
 * Output: 1,    if the file is an hidden file
 * 		   0,    else
 */
static inline int isHiddenFile(char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp)
		errorAndDie("fopen");

	return fgetc(fp) == '.';
}

/*
 * Check if a file is regular
 * Input:  path, file path
 * Output: 1,    if the file is a regular file
 * 		   0,    else
 */
static inline int isRegularFile(char *path)
{
	struct stat info;

	// Get file attributes
	if (!lstat(path, &info) < 0)
		errorAndDie("lstat");

	return S_ISREG(info.st_mode);
}

#endif
