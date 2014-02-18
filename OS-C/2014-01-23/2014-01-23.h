/*
Prova Pratica di Laboratorio di Sistemi Operativi
23 gennaio 2014
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2014.01.23.pdf

@author: Tommaso Ognibene
*/

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../const.h"

// Function declarations
static inline int isTextFile(FILE *fp);
static inline int isRegularFile(struct stat info);
static inline void openFile(const char *path, FILE **fp);
static int readFileLine(const FILE *fp, const struct stat info, const int n, char *line, int lenght);

extern void run(const char *path, const int n);

/*
 * Find a specific line in a text file.
 * Map the whole file. Search for a specific line sequentially.
 * Therefore never accesses more than one page at a time.
 * Input:
 * 			fp,     the file pointer
 * 			info,   the file attributes
 * 			n,      the specific line
 * Output:
 * 			line,   the pointer to the line
 * 			lenght, the lenght of the line
 *
 * 			1,      in case of success
 * 			0,      else
 */
static int readFileLine(const FILE *fp, const struct stat info, const int n, char *line, int lenght)
{
	char *buf;
	off_t start = -1, end = -1;
	size_t i;
	int ln, output = TRUE, count = n;

	// Pre-conditions: n >= 1
	if (n < 1) return FALSE;

	if (n == 1) start = 0;

	count--;

	buf = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, (int)fp, (int)0);
	madvise(buf, info.st_size, MADV_SEQUENTIAL);

	for (i = ln = 0; i < info.st_size && ln <= count; i++)
	{
		printf("%c\n",buf[i]);
		if (buf[i] != '\n') continue;

		printf("%d\n", ln);
		if (++ln == count)
		{
			start = i + 1;
		}
		else if (ln == count + 1)
		{
			end = i + 1;
		}
	}

	// [Case 1] File does not have lines
	if (start >= info.st_size || start < 0)
	{
		output = FALSE;
	}
	else
	{
		line = buf + start;
		lenght = ((int)end - (int)buf) * sizeof(char *);
	}

	munmap(buf, info.st_size);

	return output;
}


/*
 * Check if a file is textual
 * Input:
 * 			fp, the file pointer
 * Output:
 * 			1, if the file is a text file
 * 			0, else
 */
static inline int isTextFile(FILE *fp)
{
	int c;

	while ((c = fgetc(fp)) != EOF && c >= 1 && c <= 127);

	return c == EOF;
}

/*
 * Check if a file is hidden
 * Input:
 * 			fp, the file pointer
 * Output:
 * 			1, if the file is an hidden file
 * 			0, else
 */
static inline int isHiddenFile(FILE *fp)
{
	return fgetc(fp) == '.';
}

/*
 * Check if a file is a regular file
 * Input:
 * 			fp, the file pointer
 * Output:
 * 			1, if the file is a regular file
 * 			0, else
 */
static inline int isRegularFile(struct stat info)
{
	return S_ISREG(info.st_mode);
}

static inline void openFile(const char *path, FILE **fp)
{
	if (!(*fp = fopen(path, "r")))
	{
        perror("fopen");
        putchar('\n');
        exit(EXIT_FAILURE);
	}
}

extern void run(const char *path, const int n)
{
	FILE *fp = NULL;
	int count = 0;
	struct stat info;

	// Get file attributes
	if (!lstat(path, &info) < 0)
	{
        perror("lstat");
        putchar('\n');
        exit(EXIT_FAILURE);
	}

	// Open file
	openFile(path, &fp);

	// If it is a text file and a regular file
	if (isTextFile(fp) && isRegularFile(info))
	{
		// Get the lenght of the n-th line
		readFileLine(fp, info, n, NULL, count);
		printf("%d\n", count);
	}

	fclose(fp);
	exit(EXIT_SUCCESS);
}
