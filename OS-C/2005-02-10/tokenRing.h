/*
Prova Pratica di Laboratorio di Sistemi Operativi
10 febbraio 2005
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.02.10.pdf

Nota: variazione che prevede un gestore (Master), di un token ring
      composto da n processi figli (Slaves).

@author: Tommaso Ognibene
*/

// Dependencies
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants
#define True       1
#define False      0

// Data types
struct tokenRing
{
	pid_t Pid;
	int Token[2];
	int Output[2];
	struct tokenRing *Next, *Prev;
};

typedef struct tokenRing TokenRing;

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void testTokenRing(int n);
static inline int isNumber(char *text);

// Entry point
extern void run(int argc, char *argv[])
{
	// Pre-conditions
	if (argc != 2)
		printAndDie("The function requires one parameter to be passed in.");
	if (!isNumber(argv[1]))
		printAndDie("The parameter should be a natural number.");

	testTokenRing(atoi(argv[1]));
}

/*
 * Check whether a string is a natural number or not
 * Input:   str,  the string
 * Output:  1,    if the string is a natural number
 * 			0,    else
 */
static inline int isNumber(char *str)
{
	int i;

	i = 0;
	while (i < strlen(str) && isdigit(str[i]))
		i++;

	return i == strlen(str);
}

/*
 * Create a token ring of n processes.
 * The parent shall manage the token ring.
 * Run a test showing the main logical steps in the parent standard output.
 * Input: n, the number of processes
 */
static inline void testTokenRing(int n)
{
	const char fifoPath[] = "token.fifo";
	char token[5] = "token";
	char buffer[512];
	int i, fifo;
	pid_t pid;
	ssize_t count;
	TokenRing *it, *first, *temp;

	// First element of the token ring
	first = it = (TokenRing *) malloc(sizeof (TokenRing));

	// Create the named pipe if it does not yet exist
	if (access(fifoPath, F_OK) < 0)
		if (mkfifo(fifoPath, S_IRWXU) < 0)
			errorAndDie("mkfifo");

	for (i = 0; i < n; i++)
	{
		// Create a pipe for token communication
		if (pipe(it->Token) < 0)
			errorAndDie("pipe");

		// Create a pipe for output redirection
		if (pipe(it->Output) < 0)
			errorAndDie("pipe");

		// Fork
		pid = fork();
		if (pid < 0)
			errorAndDie("fork");

		// Child process
		if (pid == 0)
		{
			// Open named pipe for writing
			fifo = open(fifoPath, O_WRONLY);
			if (fifo < 0)
				errorAndDie("open");

			// Close output side
			if (close(it->Token[1]) < 0)
				errorAndDie("close");

			// Close input side
			if (close(it->Output[0]) < 0)
				errorAndDie("close");

			// Re-direct the standard output
			if (dup2(it->Output[1], STDOUT_FILENO) < 0)
				errorAndDie("dup2");

			while (True)
			{
				// Clear the buffer
				memset(buffer, 0, sizeof (buffer));

				// Wait for the token
				count = read(it->Token[0], buffer, sizeof (buffer));
				if (count < 0)
					errorAndDie("read");

				fprintf(stdout, "[slave #%d] Received the token from Master.\n", getpid());
				fprintf(stdout, "[slave #%d] I'm gonna do some stuff now.\n", getpid());
				fflush(stdout);
				sleep(2);
				fprintf(stdout, "[slave #%d] Giving back the token to Master.\n", getpid());
				fflush(stdout);

				// Give the token back
				count = write(fifo, token, sizeof (token));
				if (count < 0)
					errorAndDie("write");
			}

			// Close the pipes
			if (close(it->Token[0]) < 0 || close(it->Output[1]) < 0 || close(fifo) < 0)
				errorAndDie("close");

			exit(EXIT_SUCCESS);
		}

		// Close unused sides of the pipes
		if (close(it->Token[0]) < 0 || close(it->Output[1]) < 0)
			errorAndDie("close");

		it->Pid = pid;

		if (i + 1 == n)
			break;

		it->Next = (TokenRing *) malloc(sizeof (TokenRing));
		temp = it;
		it = it->Next;
		it->Prev = temp;
	}
	it->Next = first;
	first->Prev = it;

	// Open named pipe for reading
	fifo = open(fifoPath, O_RDONLY);
	if (fifo < 0)
		errorAndDie("open");

	// Manage the token ring
	it = first;
	while (True)
	{
		// Clear the buffer
		memset(buffer, 0, sizeof (buffer));

		// Assign the token
		printf("[Master] Assigning the token to slave #%d\n", it->Pid);
		count = write(it->Token[1], token, sizeof (token));
		if (count < 0)
			errorAndDie("write");

		// Redirect the children output
		for (i = 0; i < 2; i++)
		{
			count = read(it->Output[0], buffer, sizeof (buffer));
			if (count < 0)
				errorAndDie("read");

			if (write(STDOUT_FILENO, buffer, count) < 0)
				errorAndDie("write");
		}

		// Wait for the token in return
		count = read(fifo, buffer, sizeof (buffer));
		if (count < 0)
			errorAndDie("read");
		printf("[Master] Received the token from slave #%d\n", it->Pid);

		it = it->Next;
	}

	// Close the pipes
	if (close(it->Token[1]) < 0 || close(it->Output[0]) < 0 || close(fifo) < 0)
		errorAndDie("close");

	// Remove the named pipe
	if (unlink(fifoPath) < 0)
		errorAndDie("unlink");
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
