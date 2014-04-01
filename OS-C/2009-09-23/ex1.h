/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 settembre 2009
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.09.23.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants
#define KB      1024
#define True    1
#define False   0
#define Success True
#define Failure False

// Function declarations
static inline int  isFile(char *path);
static inline int  getSizeFile(char *path, off_t *size);
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void inverseArray(char *x, int n);
static inline int  inverseFile(char *pathSrc, char *pathDst, off_t chunk);

extern void run(int argc, char *argv[])
{
	// Pre-conditions
	if (argc != 3)
		printAndDie("The function requires two parameters to be passed in.");

	// Inverse file
	if (!inverseFile(argv[1], argv[2], 4 * KB))
		printAndDie("Failed to inverse the file. Something fishy about the parameters.");
}

/*
 * Inverse a file chunk by chunk.
 * Input:  pathSrc, path to the source file
 *         pathDst, path to the destination file
 *         chunk,   chunk of bytes
 * Output: 1, Success
 *         0, Failure
 */
static inline int inverseFile(char *pathSrc, char *pathDst, off_t chunk)
{
	off_t offset, size;
	char *buffer;
	int src, dst;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // Full access

	// Pre-conditions: source exists, destination does not
	if (!getSizeFile(pathSrc, &size) || isFile(pathDst))
		return Failure;

	// Open source file
	src = open(pathSrc, O_RDONLY);
	if (src < 0)
		errorAndDie("open");

	// Create destination file
	dst = creat(pathDst, mode);
	if (dst < 0)
		errorAndDie("creat");

	// Set size of destination
	if (ftruncate(dst, size) < 0)
		errorAndDie("ftruncate");

	// Define the content buffer
	buffer = (char *) malloc(chunk * sizeof (char));

	// Inverse the file chunk by chunk
	for (offset = 0; offset + chunk <= size; offset += chunk)
	{
		// Clear buffer
		memset(buffer, 0, chunk);

		// Read from source
		if (pread(src, buffer, chunk, offset) < 0)
			errorAndDie("pread");

		// Inverse data
		inverseArray(buffer, chunk);

		// Write to destination
		if (pwrite(dst, buffer, chunk, size - offset - chunk) < 0)
			errorAndDie("pwrite");
	}

	// Check if there is a last minor chunk to process
	if (offset != size)
	{
		// Clear buffer
		memset(buffer, 0, chunk);

		// Read from source
		if (pread(src, buffer, size - offset, offset) < 0)
			errorAndDie("pread");

		// Inverse data
		inverseArray(buffer, size - offset);

		// Write to destination
		if (pwrite(dst, buffer, size - offset, 0) < 0)
			errorAndDie("pwrite");
	}

	// Close file descriptors
	if (close(src) < 0 || close(dst) < 0)
		errorAndDie("close");

    // Garbage collection
	free(buffer);

	return Success;
}

/*
 * Invert the elements of an array. In-place, with swapping.
 * Input:  x, array
 *         n, array size
 */
static inline void inverseArray(char *x, int n)
{
	int left, right, temp;

	right = n - 1;
	for (left = 0; left < n / 2; left++)
	{
		temp = x[left];
		x[left] = x[right];
		x[right] = temp;
		right--;
	}
}

/*
 * Check if a file exists.
 * Input:  path, file path
 * Output: 1,    if the file exists
 * 		   0,    else
 */
static inline int isFile(char *path)
{
	FILE *file = fopen(path, "r");

	if (file)
	{
		fclose(file);
		return True;
	}

	return False;
}

/*
 * Check if a file is a regular file.
 * If yes, return its size in bytes.
 * Input:  path, file path
 * Output: 1,    if it is a regular file
 * 		   0,    else
 *
 * 		   size, its size
 */
static inline int getSizeFile(char *path, off_t *size)
{
	struct stat info;

	if (isFile(path))
	{
		if (stat(path, &info) < 0)
			errorAndDie("stat");

		if (info.st_mode & S_IFREG)
		{
			*size = info.st_size;
			return True;
		}
	}

	return False;
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
