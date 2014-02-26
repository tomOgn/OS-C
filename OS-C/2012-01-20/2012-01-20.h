/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 gennaio 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012-01-20.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Size type used with offsets and read/write operations;
 * To avoid (the overhead of) successive lseek operations
 * with nul gaps of 2 GB or more on LP64 64 bit (Linux) systems:
 * compile with e.g.: -DSPARSEFILE_SIZETYPE=off_t
 * (where blksize_t, size_t, off_t, etc are all long long int).
 */
#ifndef SPARSEFILE_SIZETYPE
#define SPARSEFILE_SIZETYPE int
#endif

/*
 * For support for files of 2 GB and more on 32 bit (Linux) systems:
 * compile with: -DSPARSEFILE_LSEEK=lseek64 -D_LARGEFILE64_SOURCE
 * ; although we take care not to overflow in the skip parameter,
 * 32 bit lseek will likely fail without these options
 * with EOVERFLOW ('Value too large for defined data type')
 * as soon as the 2 GB file size limit is hit.
 */
#ifndef SPARSEFILE_LSEEK
#define SPARSEFILE_LSEEK lseek
#endif

/*
 * Print error message and exit
 * Input: msg, the error message
 */
static inline void errorAndDie(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

/* sparse -- copy in to out while producing a sparse file */
int sparse(int fdin, int fdout)
{
	static char const skipbyte = '\0';
	struct stat st;
	SPARSEFILE_SIZETYPE blockSize, skip, nskip, i, eof, n, i;
	char* buf;
	int sum;

	// Get the block size
	if (fstat(fdout, &st) == -1)
		errorAndDie("fstat");
	blockSize = st.st_blksize;

	// Allocate buffer
	buf = (char *) malloc(blockSize * sizeof(char *));
	if (!buf)
		errorAndDie("malloc");

	for (eof = 0, skip = 0;;)
	{
		sum = 0;
		// Read one block
		for (i = 0; i < blockSize; i += n)
		{
			n = read(fdin, &buf[i], blockSize - i);
			if (n == -1)
				errorAndDie("read");

			sum += (int) buf[i];

			if (n == 0)
			{
				eof++;
				break;
			}
		}

		// Check if we can skip this part
		nskip = 0;
		if (i == blockSize)
		{
			// Linear search for a byte other than skipbyte */
			for (n = 0; n < blockSize; n++)
			{
				if (buf[n] != skipbyte)
				{
					break;
				}
			}
			if (n == blockSize)
			{
				nskip = skip + blockSize;
				if (nskip > 0)
				{ /* mind 31 bit overflow */
					skip = nskip;
					continue;
				}
				nskip = blockSize;
			}
		}

		/* do a lseek over the skipped bytes */
		if (skip != 0) {
			/* keep one block if we got eof, i.e. don't forget to write the last block */
			if (i == 0) {
				/* note that the following implies using the eof flag */
				skip -= blockSize;
				i += blockSize;
				/* we don't need to zero out buf since the last block was skipped, i.e. zero */
			}

			i = SPARSEFILE_LSEEK(fdout, skip, SEEK_CUR);
			if (i == -1)
				errorAndDie("lseek");

			skip = 0;
		}
		/* continue skipping if just skipped near overflow */
		if (nskip != 0)
		{
			skip = nskip;
			continue;
		}

		// Write exactly i
		for (n = 0; n < i; n += i)
		{
			i = write(fdout, &buf[n], i - n);
			if (
				i == -1 || /* error */
				i == 0 /* can't write?? */
			) {
				perror("write");
				free(buf);
				return -1;
			}
		}

		if (eof)
			break;
	}

	free(buf);
	return 0;
}

/* main -- program entry point */
int main(int argc, char** argv) {
	/* usage */
	if (argc != 1) {
		fprintf(stderr, "usage: %s <infile >outfile\n", argv[0]);
		fprintf(stderr, "or: ... | %s >outfile\n", argv[0]);
		return 1;
	}

	/* read from stdin (0), write to stdout (1) */
	if (sparse(0, 1) == -1) {
		perror(argv[0]); /* just repeat error message here, including causing program name */
		return 1;
	}

	return 0;
}


extern void run(int argc, char *argv[])
{


	exit(EXIT_SUCCESS);
}
