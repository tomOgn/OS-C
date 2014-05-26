/*
Prova Pratica di Laboratorio di Sistemi Operativi

20 giugno 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.05.29.pdf

@author: Tommaso Ognibene
*/

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <unistd.h>

#define BUFFSIZE 10
#define True 1

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void printEventFD(int argc, char *argv[]);

extern void run(int argc, char *argv[])
{
	printEventFD(argc, argv);
}

static void printEventFD(int argc, char *argv[])
{
	int efd, pid;

	if ((efd = eventfd(0 ,EFD_SEMAPHORE)) < 0)
		errorAndDie("eventfd");

	if ((pid = fork()) < 0)
		errorAndDie("fork");

	if (pid > 0)
	{
		while (True)
		{
			uint64_t a;
			scanf("%lld", &a);
			write(efd, &a, sizeof (a));
		}
	}
	else
	{
		while (True)
		{
			uint64_t b;
			read(efd, &b, sizeof (b));
			printf("x\n");
		}
	}
}

extern void testEventFD(int argc, char *argv[])
{
	int efd, j;
	uint64_t u;
	ssize_t s;

	efd = eventfd(0, EFD_NONBLOCK);
	if (efd < 0)
	   errorAndDie("eventfd");

	switch (fork())
	{
	case 0:
		for (j = 1; j < argc; j++)
		{
			printf("Child writing %s to efd\n", argv[j]);
			u = strtoull(argv[j], NULL, 0);
			s = write(efd, &u, sizeof(uint64_t));
			if (s != sizeof(uint64_t))
				errorAndDie("write");
		}
		printf("Child completed write loop\n");

		exit(EXIT_SUCCESS);
	default:
		sleep(2);

		printf("Parent about to read\n");
		s = read(efd, &u, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
			errorAndDie("read");
		printf("Parent read %llu (0x%llx) from efd\n", (unsigned long long) u, (unsigned long long) u);
		exit(EXIT_SUCCESS);

	case -1:
		errorAndDie("fork");
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
