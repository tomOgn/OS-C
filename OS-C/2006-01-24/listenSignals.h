/*
Prova Pratica di Laboratorio di Sistemi Operativi
24 gennaio 2006
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2006-01-24.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);

extern void run(int argc, char *argv[])
{
	if (argc != 1)
		printAndDie("Wrong number of parameters.");

	listenSignals();
}

/*
 * Signal handler. Just print information about the signal.
 */
static void printHandler(int sig, siginfo_t *info, void *data)
{
    printf("Signal: %d. Process ID: %d. User ID: %d\n", sig, info->si_pid, info->si_uid );
}

/*
 * Wait for signals, using an handler.
 */
static void listenSignals()
{
    struct sigaction sa;

    memset(&sa, 0, sizeof (struct sigaction));
    sa.sa_sigaction = &printHandler;
    sa.sa_flags |= SA_SIGINFO;

    sigemptyset(&sa.sa_mask);
    sigaction(NULL, &sa, NULL);

    while (True)
        sleep(1);
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
