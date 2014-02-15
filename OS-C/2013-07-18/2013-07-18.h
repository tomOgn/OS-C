/*
Prova Pratica di Laboratorio di Sistemi Operativi
18 luglio 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.07.18.pdf

@author: Tommaso Ognibene
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Constants
#define MaxNumArguments 10
#define MaxLenArguments 50
#define MaxLenPath 100
#define MaxLenLine MaxLenArguments * MaxNumArguments
#define MaxLenCommands 500

// Structure for a process element
typedef struct processElem
{
	char CommandLine[MaxLenCommands];
	pid_t Pid;
	struct processElem *Next;
} ProcessElem;

// Function prototypes
ProcessElem *appendElement    (ProcessElem *tail);
char        *findCommand      (pid_t pid);
void         countTokens      (const char *string, const char delimiter, int *count, int *longest);
int          tokenize         (char *string, char delimiter, char ***tokens, int *count);
void         readMultipleLines(void);
void         runProcesses     (void);

// Global variables
static ProcessElem *ProcessList  = NULL;
int                 ProcessCount = 0;

// Given a PID retrieve the relative command
char *findCommand(pid_t pid)
{
	ProcessElem *process;
	for(process = ProcessList; process->Pid != pid; process = process->Next);
	return (process)? (process->CommandLine) : (NULL);
}

// Create a new element
ProcessElem *appendElement(ProcessElem *tail)
{
	ProcessElem *newElem = (ProcessElem *)malloc(sizeof(ProcessElem));
	newElem->Next = NULL;
	(tail)? (tail->Next = newElem) : (tail = newElem);
	return newElem;
}

// Garbage collection for array of chars
void freeCharArray(char **data, int count)
{
	if(!data) return;
	int i;
	for(i = 0; i < count; i++)
		free(data[i]);
	free(data);
}

/*
 * Count tokens and find the longest one
 * Example:
 *    Input:
 *       string = "ping -c 4 google.com"
 *       delimiter = ' '
 *    Output:
 *       count = 4
 *       longest = 10
 */
void countTokens(const char *string, const char delimiter, int *count, int *longest)
{
	int iterator, lenght;
	lenght = *longest = *count = 0;

	// Count the first (n - 1) tokens
	for(iterator = 0; string[iterator]; iterator++)
	{
		if(string[iterator] == delimiter)
		{
			if(*longest < lenght)
				*longest = lenght;
			lenght = 0;
			(*count)++;
		}
		lenght++;
	}

	// Count the last token
	if(string[iterator - 1] != delimiter)
	{
		lenght--;
		if(*longest < lenght)
			*longest = lenght;
		(*count)++;
	}
}

/*
 * Tokenize a string accordingly to a given delimiter
 * Example:
 *    Input:
 *       string = "ping -c 4 google.com"
 *       delimiter = ' '
 *    Output:
 *       tokens = ["ping", "-c", "4", "google.com"]
 *       count = 4
 */
int tokenize(char *string, char delimiter, char ***tokens, int *count)
{
	if(!(tokens && string && count)) return 0;

	int token, i, j, longest;
	*count = longest = 0;

	countTokens(string, delimiter, count, &longest);

	// Allocate space for array of strings
	*tokens = (char **)malloc(*count * sizeof(char *));

	if(!*tokens)
	{
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	// Allocate space for each string in the array and tokenize it
	i = 0;
	for(token = 0; token < *count; token++)
	{
		(*tokens)[token] = (char *)malloc(sizeof(char) * (longest + 1));

		j = 0;
		while(string[i] != delimiter && string[i])
		{
			(*tokens)[token][j] = string[i];
			j++;
			i++;
		}

		// Null-terminated string
		(*tokens)[token][j] = (char)0;
		i++;
	}

	return 1;
}

// Read multiple input lines
void readMultipleLines(void)
{
	ProcessElem *process = NULL;
	char input[MaxLenLine];
	char *iterator;

	printf("Enter commands. Press enter on blank line to start them.\n");
	while(1)
	{
		// Get input string
		if(!fgets(input, MaxLenLine, stdin))
		{
			perror("fgets()");
			exit(EXIT_FAILURE);
		}

		// Skip blank lines
		for(iterator = input; *iterator == ' '; iterator++);

		// Check if enter was pressed on blank line
		if(*iterator == '\n') break;

		// Append a new input element
		if(!(ProcessList))
			process = ProcessList = appendElement(ProcessList);
		else
		{
			process->Next = appendElement(process);
			process = process->Next;
		}
		ProcessCount++;

		// Eliminate "\n" from the input string
		iterator = strtok(iterator, "\n");

		strcpy(process->CommandLine, iterator);
	}
}

/*
 * Run processes ->
 * Print when a process terminate
 */
void runProcesses(void)
{
	pid_t pid;
	int status, terminated, count;
	char path[MaxLenPath];
	ProcessElem *process = ProcessList;
	char **command = NULL;
	terminated = count = 0;

	// Start processes
	while(process)
	{
		// Tokenize command line
		tokenize(process->CommandLine, ' ', &command, &count);

		// Create path
		snprintf(path, sizeof path, "%s%s", "/bin/", command[0]);

		// Create child process
		pid = fork();

		// Check PID
		switch(pid)
		{
		// Error
		case -1:
			perror("fork()");
			exit(EXIT_FAILURE);
		// Child process
		case 0:
			// Execute command
			if (execv(path, command) == -1)
			{
				perror("execv()");
				exit(EXIT_FAILURE);
			}
			break;
		// Parent process
		default:
			process->Pid = pid;
		}
		process = process->Next;
	}

	// Wait processes
	do
	{
		// Check if a child process has terminated
		pid = waitpid(-1, &status, WNOHANG);
		switch(pid)
		{
		// Error
		case -1:
			perror("waitpid()");
			exit(EXIT_FAILURE);
		// All child processes are still running
		case 0:
			break;
		// A child process has terminated
		default:
			terminated++;
			printf("Finished #%d: %s\n", terminated, findCommand(pid));
		}
	}
	while(terminated < ProcessCount);

	exit(EXIT_SUCCESS);
}

// Entry point
int run(int argc, char *argv[])
{
	// Get input
	readMultipleLines();

	// Set output
	runProcesses();

    return EXIT_SUCCESS;
}
