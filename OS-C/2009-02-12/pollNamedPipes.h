/*
Prova Pratica di Laboratorio di Sistemi Operativi
12 febbraio 2009
Esercizio 2

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2009.02.12.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants
#define BufferSize 1024

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void pollNamedPipes(int n, char *name[]);

extern void run(int argc, char *argv[])
{
	// Sanity check
	if (argc != 3)
		printAndDie("The function requires two parameters to be passed in.");

	char *name[2] = { argv[1], argv[2] };
	pollNamedPipes(name, 2);
}

/*
 * Multiplex I/O over a set of named pipes.
 * Loop until there is at least one stream active.
 * Input: name, the named pipes
 *        n,    the number of named pipes
 */
static inline void pollNamedPipes(char *name[], int n)
{
	int	i, fd, count, streams;
	struct pollfd fds[n];
	char buffer[BufferSize];

	for (i = 0; i < n; i++)
	{
		// Create a named pipe
		if ((mkfifo(name[i], S_IRWXU)) < 0)
			errorAndDie("mkfifo");

		// Open a stream
		fd = open(name[i], O_RDONLY);
		if (fd < 0)
			errorAndDie("open");

		// Initialize a file descriptors set
		fds[i].fd = fd;
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	// Loop until there are >= 1 running streams
	streams = n;
	while (streams > 0)
	{
		// Wait until an event has occurred
		if (poll(fds, n, -1) < 0)
			errorAndDie("poll");

		for (i = 0; i < n; i ++)
		{
			// Check for available data
			if ((fds[i].revents & POLLIN) != 0)
			{
				// Clear the buffer
				memset(buffer, 0, sizeof (buffer));				
				
				// Fill the buffer
				count = read(fds[i].fd, &buffer, sizeof (buffer));
				if (count < 0)
					errorAndDie("read");

				// Deliver the buffer
				if (write(STDOUT_FILENO, buffer, count) < 0)
					errorAndDie("write");
			}

			// Check if the device has been disconnected
			if ((fds[i].revents & POLLHUP ) != 0)
			{
				printf("The device %s has been disconnected\n", name[i]);
				fds[i].fd = -1;
				streams--;
			}
		}
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
