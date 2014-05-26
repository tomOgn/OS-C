/*
Prova Pratica di Laboratorio di Sistemi Operativi
27 giugno 2005
Esercizio 1 & 2

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.06.27.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Function declarations
static inline void checkError(int error, const char *msg);
static inline void printAndDie(const char *msg);
static inline void linearAccelerator(int n);
static inline void terminateExperiment();
static inline void errorAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	// Sanity check
	if (argc != 2)
		printAndDie("The function requires only one paramenter to be passed in.");

	int n = atoi(argv[1]);
	if (n < 6)
		printAndDie("The number of nodes should be greater than 5.");

	linearAccelerator(n);
}

static inline void linearAccelerator(int n)
{
	pid_t pid;
	int value, i, particles, emitter, receiver;
	int fd[2];
	int trigger[2];
	char particle[1] = { 'p' };
	char buffer[1];
	time_t startTime, stopTime;
    fd_set set;
    struct timeval timeout;
    int ready;

    FD_ZERO(&rfds);
    FD_SET(0, &set);



	// [1] Create the Linear Accelerator

	// Create a pipe used as a trigger
	if (pipe(trigger) < 0)
		errorAndDie("pipe");

	// Close input side
	if (close(trigger[0]) < 0)
		errorAndDie("close");

	// Create the child processes
	for (i = 0; i < n; i++)
	{
		// Create a pipe
		if (pipe(fd) < 0)
			errorAndDie("pipe");

		// Fork the process
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		// Child
		if (pid == 0)
		{
			while (True)
			{
			    // Wait up to one seconds
			    timeout.tv_sec = 1;
			    timeout.tv_usec = 0;

			    ready = select(1, &set, NULL, NULL, &timeout);

			    if (ready < 0)
			        perrorAndDie("select");
			    else if (ready)
			    {

			    }
			        printf("Data is available now.\n");
			        /* FD_ISSET(0, &rfds) will be true. */
			    else
			        printf("No data within five seconds.\n");

				// Wait a message from daddy
				if (read(fd[0], buffer, sizeof (buffer)) < 0)
					errorAndDie("read");

				// Write back to daddy
				if (write(fd[1], buffer, sizeof (buffer)) < 0)
					errorAndDie("write");

				// To die or not to die?
				if (rand() % 2)
				{
					// Close pipe
					if (close(fd[0]) < 0 || close(fd[1]) < 0)
						errorAndDie("close");

					// Die
					printAndDie("Farewell Cruel World!");
				}
			}
		}
	}
	// Daddy


				// Emitter
				if (i == emitter)
				{
					// The write is repeated if a signal has interfered
					do
						value = write(fd[i][1], particle, 1);
					while (value < 0 && errno == EINTR);

					// If there is only one node left, the experiment is terminated
					if (value < 0 && errno == EPIPE)
						terminateExperiment();

					checkError(value, "write");

					// Wait one second before firing another particle
					sleep(1);
				}
				// Transport
				else if (i < receiver)
				{
					// The read is repeated if a signal has interfered
					do
						value = read(fd[i - 1][0], particle, 1);
					while (value < 0 && errno == EINTR);
					checkError(value, "read");

					// If the previous node is dead, this node becomes the emitter
					if (value == 0)
						emitter = i;

					// 50% of chances to continue
					if (rand() % 2)
					{
						// The write is repeated if a signal has interfered
						do
							value = write(fd[i][1], particle, 1);
						while (value < 0 && errno == EINTR);

						// If the next node is dead, this node becomes the receiver
						if (value < 0 && errno == EPIPE)
						{
							receiver = i;

							// If there is only one node left, the experiment is terminated
							if (receiver == emitter)
								terminateExperiment();
						}
						else
							checkError(value, "write");
					}
				}
				// Receiver
				else
				{
					// The read is repeated if a signal has interfered
					do
						value = read(fd[i - 1][0], particle, 1);
					while (value < 0 && errno == EINTR);
					checkError(value, "read");

					// If there is only one node left, the experiment is terminated
					if (value == 0)
						terminateExperiment();

					time(&stopTime);
					particles++;
					printf("Particles that have been successfully received:\n");
					printf("\tTotal Number       = %d\n", particles);
					printf("\tAverage Per Second = %.2f\n", particles / difftime(stopTime, startTime));
				}
			}
		}

	}
	i = 0;
	do
	{
		// Create a pipe
		if (pipe(fd) < 0)
			errorAndDie("pipe");

		// Fork the process
		if ((pid = fork()) < 0)
			errorAndDie("fork");

		// Child
		if (pid == 0)
		{
			// Close output side
			if (close(fd[1]) < 0)
				errorAndDie("close");

			i++;
		}
		// Parent
		else
		{
			// Close input side
			if (close(fd[0]) < 0)
				errorAndDie("close");
		}
	} while (i < n - 1 && pid == 0);

	// [3] Execute the Linear Accelerator
	particles = 0;
	emitter = 0;
	receiver = n - 1;
	time(&startTime);

	}
}

/*
 * Retrieve the output of the given named pipes.
 * Input: name, the pipe names
 *        n,    the number of pipes
 */
static inline void outputNamedPipe(char *name[], int n)
{
    int *fifos;
    int ready, highest, buff_count, count, i, error;
    fd_set set, tmp; // set of file descriptors
    char buff[BufferSize];

	// Clear the set
	FD_ZERO(&set);

	// Populate the set
	fifos = (int *) malloc(sizeof(int) * n);
	for (i = 0; i < n; i++)
	{
		printf("%s\n", name[i]);
		fifos[i] = open(name[i], O_RDWR | O_NONBLOCK);
		checkError(fifos[i], "open");

		// Add the file descriptor to the set
		FD_SET(fifos[i], &set);
	}

	// Get the highest-numbered file descriptor + 1
	highest = max(fifos, n) + 1;

	// Save a copy of the original set
	memcpy((void *) &tmp, (void *) &set, sizeof (fd_set));

	while (True)
	{
		// Block the program until I/O is ready on one or more of the file descriptors
		ready = select(highest, &set, NULL, NULL, NULL);
		checkError(ready, "select");

		for (i = 0, count = 0; count < ready || i < n; i++)
		{
			// Test if the file descriptor is part of the set
			if (!FD_ISSET(fifos[i], &set)) continue;

			count++;
			printf("Fifo: %d\n", i);
			buff_count = read(fifos[i], &buff, BufferSize);
			checkError(buff_count, "read");

			printf("Read %d bytes\n", buff_count);
			error = write(STDOUT_FILENO, buff, sizeof(char) * (unsigned int) buff_count);
			checkError(error, "write");
		}

		// Restore the original set
		memcpy((void *) &set,(void *) &tmp, sizeof(fd_set));
	}
}



int
main(void)
{
    fd_set set;
    struct timeval timeout;
    int count;

    FD_ZERO(&rfds);
    FD_SET(0, &set);

    // Wait up to one seconds
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    count = select(1, &set, NULL, NULL, &timeout);

    if (count < 0)
        perrorAndDie("select");
    else if (count)
        printf("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
    else
        printf("No data within five seconds.\n");

    exit(EXIT_SUCCESS);
}



static inline void childBehaviour()
{

}

static inline void terminateExperiment()
{
	printf("Emitter and receiver are the same node. The experiment is terminated.\n");
	exit(EXIT_SUCCESS);
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
 * Check if the standard error code occured (return value < 0).
 * If yes, print the error message and exit.
 * Input: error, the return value of a system call
 *        msg,   the error message
 */
static inline void checkError(int error, const char *msg)
{
	if (error >= 0) return;

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
