/*
Prova Pratica di Laboratorio di Sistemi Operativi
22 giugno 2011
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2011.06.22.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Function declarations
void signalHandler(int sig);

// Global variables
static int inputSignal;

extern void run(int argc, char *argv[])
{
	sigset_t new_mask;

	if (signal(inputSignal, signalHandler) == SIG_ERR)
	{

	}

	/* initialize the new signal mask */
	sigemptyset(&new_mask);
	sigaddset(&new_mask,SIGCHLD);

	/* wait for any signal except SIGCHLD */
	sigsuspend(&new_mask);

	/* we only expect to get control here if sigsuspend was */
	/* interrupted by a signal for which a signal handler   */
	/* was called and only if that signal handler returned  */

	/* initialize the new signal mask */
	sigfillset(&new_mask);
	sigdelset(&new_mask,SIGUSR1);

	/* wait for ONLY a SIGUSR1 signal */
	sigsuspend(&new_mask);

	exit(EXIT_SUCCESS);
}

void handleSignal(int signal)
{
    const char *signal_name;
    sigset_t pending;

    if (signal == inputSignal)
    {

    }

}
