/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 giugno 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.06.22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../const.h"

#define MAX_LEN 		1024
#define MAX_NUM_ARGS 	1024
#define MAX_LEN_ARG	 	1024

typedef struct
{
	int clientPID, serverPID;
	char args[MAX_NUM_ARGS][MAX_LEN_ARG];
	int count;
} sharedRegion;

static sharedRegion *ptr;

/*
 * Print error message and exit
 * Input: msg, the error message
 */
static inline void errorAndDie(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

extern void run(int argc, char *argv[])
{
	int i, fd;
	const char *name = "/VeniVidiPrinti";

	// Establish a connection between shared memory object and file descriptor
	fd = shm_open(name, O_RDWR, 0666);
	if (fd == -1)
		errorAndDie("shm_open");

	// Maps file descriptor into memory
	ptr = mmap(0, sizeof(sharedRegion), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		errorAndDie("mmap");
	close(fd);

	for (i = 0; i < argc; i++)
		strcpy(ptr->args[i], argv[i]);
	ptr->count = argc;

	kill(ptr->serverPID, SIGUSR1);

    exit(EXIT_SUCCESS);
}
