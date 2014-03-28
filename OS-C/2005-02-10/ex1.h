/*
Prova Pratica di Laboratorio di Sistemi Operativi
10 febbraio 2005
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2005.02.10.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#define FIFO_NAME "/tmp/my_fifoxx"
#define BUFFER_SIZE PIPE_BUF
#define BSZ 20
#define MGC 12
#define MaxLenArr 128 // Max length array
#define MaxLenStr 256 // Max length string
// Constants

// Data types
typedef struct
{
	// File path
	char *pth;

	// Command
	char *cmd[MaxLenArr];

	// Environment variables
	char *env[2];
} Process;

struct tokenRing
{
	pid_t Pid;
	int Fd[2];
	struct tokenRing *Next;
};

typedef struct tokenRing TokenRing;

typedef struct
{
	TokenRing *Me;
} sharedRegion;

static sharedRegion *ptr;

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static inline void runProcess(Process *process, int n);
static void try(void);
static void do_msg_send(int pip[]);
static void initializeTokenRing(int n);
static inline sharedRegion *setSharedMemory(const char *name);

extern void run(int argc, char *argv[])
{
	initializeTokenRing(5);
}

/*
 * Inizialize a shared memory object.
 * Input:  name, shared memory object' name
 * Output: pointer to the shared memory object
 */
static inline sharedRegion *setSharedMemory(const char *name)
{
	int fd;

	// Create a shared memory object
	fd = shm_open(name, O_CREAT | O_TRUNC | O_RDWR, 0666);
	if (fd < 0)
		errorAndDie("shm_open");

	// Set shared memory size
	if (ftruncate(fd, sizeof (sharedRegion)) < 0)
		errorAndDie("ftruncate");

	// Establish a mapping between process' address space and shared memory
	ptr = mmap(0, sizeof (sharedRegion), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		errorAndDie("mmap");

	return ptr;
}

static void initializeTokenRing(int n)
{
	int fd[n][2];
	pid_t pid;
	int i, count;
	char buffer[32];
	TokenRing *it;
	FILE *stream;
	int output[n][2];
	struct timeval time;

	// Initialize shared memory object
	ptr = setSharedMemory("SharedMemory");
	ptr->Me = (TokenRing *) malloc(sizeof (TokenRing));
	ptr->Me->Pid = getpid();

	// Create a new pipe
	if (pipe(ptr->Me->Fd) < 0)
		errorAndDie("pipe");

//	// Create named pipe
//	if ((mkfifo("token.fifo", S_IRWXU)) < 0)
//		errorAndDie("mkfifo");

	it = ptr->Me;
	for (i = 0; i < n; i++)
	{
		// Create a new element
		it->Next = (TokenRing *) malloc(sizeof (TokenRing));
		it = it->Next;
		it->Pid = (pid_t) atoi(buffer);

		// Create a new pipe
		if (pipe(fd[i]) < 0)
			errorAndDie("pipe");

//		// Create a new pipe
//		if (pipe(output[i]) < 0)
//			errorAndDie("pipe");

		// Fork
		pid = fork();
		if (pid < 0)
			errorAndDie("fork");

		// Child process
		if (pid == 0)
		{
//			/* Re-direct the standard output into the pipe.
//			 * A loop is needed to allow for the possibility of being interrupted by a signal.
//			 */
//			while ((dup2(output[i][1], STDOUT_FILENO) < 0) && (errno == EINTR));
//			close(output[i][1]);
//			close(output[i][0]);

//			// Re-direct the standard output into the pipe
//			if (dup2(output[i][1], STDOUT_FILENO) < 0)
//				errorAndDie("dup2");
//			close(output[i][0]);

			// Close input side of pipe
			close(fd[i][0]);

			// Write on the pipe
			sprintf(buffer, "%d", (int) getpid());
			if (write(fd[i][1], buffer, 32) < 0)
				errorAndDie("write");

			// Open the named pipe
			stream = fopen("token.fifo", "rw");
			if (!stream)
				errorAndDie("fopen");

			// Read from the named pipe
			count = fread(buffer, sizeof (char), 32, stream);
			if(ferror(stream))
				errorAndDie("fread");

//		    fd_set rfds; // file descriptor sets for the select function. It is actually a bit array.
//		    int retval, len;
//		    char buff[255] = {0};
//
//		    // Watch stdin (fd 0) to see when it has input
//
//		    while (TRUE)
//		    {
//			    // Initialize the file descriptor set
//			    FD_ZERO(&rfds);
//
//			    // Add file descriptor to the file descriptor set
//			    FD_SET(output[i][0], &rfds);
//
//			    // Wait up to 1 minute
//				time.tv_sec = 60;
//				time.tv_usec = 0;
//
//				/* Block the calling process until:
//				 *    - there is activity on any of the specified sets of file descriptors; OR
//				 *    - the timeout period has expired */
//				retval = select(output[i][0] + 1, &rfds, NULL, NULL, &time);
//
//			    if (retval < 0)
//			    	errorAndDie("select");
//
//			    // Input is available
//			    if (FD_ISSET(0, &rfds))
//			    {
//
//			    }
//			    else
//			    	printAndDie("An unexpected delay occured.\n");
//
//
//		    }

			printf("I got this message: %s\n", buffer);

			if (atoi(buffer) == (int) getpid())
			{
				printf("It's my turn. Yuppy!\n");

				// Doing some stuff
				sleep(1);

				ptr->Me = ptr->Me->Next;

				// Perform a write
				sprintf(buffer, "%d", (int) ptr->Me->Next);
				count = fwrite(buffer, 1, strlen(buffer), stream);
			}

			exit(EXIT_SUCCESS);
		}

//		// Close output side of pipe
//		close(output[i][1]);

		// Close output side of pipe
		close(fd[i][1]);

		// Read on the pipe
		count = read(fd[i][0], buffer, sizeof (buffer));

		if (count < 0)
			errorAndDie("read");
	}

	it->Next = ptr->Me;
	it = ptr->Me;

	while (it->Next != ptr->Me)
	{
		printf("Process = %d\n", it->Pid);
		it = it->Next;
	}

	// Perform a write
	stream = fopen("token.fifo", "rw");
	sprintf(buffer, "%d", (int) ptr->Me->Next);
	count = fwrite(buffer, 1, strlen(buffer), stream);

	fd_set rfds;

	// Read on the pipe
	for (i = 0; i < n; i++)
	{
		FD_ZERO(&rfds);
		FD_SET(output[i][0], &rfds);

		// Wait for changes
		select(output[i][0], &rfds, NULL, NULL, NULL);
		if (FD_ISSET(output[i][0], &rfds))
		{
			while ((count = read(output[i][0], buffer, 32)) > 0)
			{
				if (write(1, buffer, count) < 0)
					errorAndDie("write");

				if (memset(buffer, 0, 32) < 0)
					errorAndDie("memset");
			}
		}

		// Close input side of pipe
		close(output[i][0]);
	}
}

/*
 * Run n copies of a process.
 * Wait until all of them terminate.
 * Input: process, data structure containing:
 *           pth, file to run
 *           cmd, list of arguments
 *           env, environment variables
 *        n,       number of copies
 */
static inline void runProcess(Process *process, int n)
{
	pid_t pid;
	int started, terminated;
	char buf[4096];
	ssize_t count;
	int **fd;
	extern char **environ;

	// Initialize file descriptors to be used for the pipeline
	fd = (int **) malloc(sizeof (int **) * n);
	for (started = 0; started < n; started++)
		fd[started] = (int *) malloc (sizeof(int *) * 2);

	// For each copy
	for (started = 0; started < n; started++)
	{
		// Create a new pipe
		if (pipe(fd[started]) < 0)
			errorAndDie("pipe");

		// Fork
		pid = fork();
		if (pid < 0)
			errorAndDie("fork");

		// Child process
		if (pid == 0)
		{
			// Close input side of pipe
			close(fd[started][0]);

			// Re-direct the standard output into the pipe
			if (dup2(fd[started][1], STDOUT_FILENO) < 0)
				errorAndDie("dup2");

			// Set the environment variable
			sprintf(process->env[0], "%s=%d", "NCOPIA", started);
		    environ = process->env;

			// Execute command
			if (execvp(process->pth, process->cmd) < 0)
				errorAndDie("execve");

			exit(EXIT_SUCCESS);
		}

		// Close output side of pipe
		close(fd[started][1]);

		// Read on the pipe
		while ((count = read(fd[started][0], buf, sizeof(buf))) > 0)
		{
			if (write(1, buf, count) < 0)
				errorAndDie("write");

			if (memset(buf, 0, 10) < 0)
				errorAndDie("memset");
		}

		if (count < 0)
			errorAndDie("read");
	}

	// Wait processes
	for (terminated = 0; terminated < started; terminated++)
		if (wait(NULL) < 0)
			errorAndDie("wait");

	exit(EXIT_SUCCESS);
}

static void tryFIFO1()
{
	int flags, ret_value, c_status;
	pid_t pid;
	size_t n_elements;
	char char_ptr[32];
	char str[] = "string for fifo ";
	char fifoname[] = "temp.fifo";
	FILE *rd_stream, *wr_stream;

	// Create named pipe
	if ((mkfifo(fifoname, S_IRWXU)) < 0)
		errorAndDie("mkfifo");

	// Fork process
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		// Issue fopen for write
		wr_stream = fopen(fifoname, "w");
		if (!wr_stream)
		{
			printf("In child process\n");
			printf("fopen returned a NULL, expected valid stream\n");
			exit(100);
		}

		// Perform a write
		n_elements = fwrite(str, 1, strlen(str), wr_stream);
		if (n_elements != (size_t) strlen(str))
		{
			printf("Fwrite returned %d, expected %d\n",
			(int)n_elements,strlen(str));
			exit(101);
		}
		exit(0);
	}

	// Parent process

	// Issue fopen for read
	rd_stream = fopen(fifoname, "r");
	if (!rd_stream)
	{
		printf("In parent process\n");
		printf("fopen returned a NULL, expected valid pointer\n");
		exit(2);
	}

	// Get current flag settings of file
	if ((flags = fcntl(fileno(rd_stream), F_GETFL)) < 0)
	{
		printf("fcntl returned -1 for %s\n",fifoname);
		exit(3);
	}

	// Clear O_NONBLOCK and reset file flags
	flags &= (O_NONBLOCK);
	if ((fcntl(fileno(rd_stream), F_SETFL, flags)) < 0)
	{
		printf("\nfcntl returned -1 for %s", fifoname);
		exit(4);
	}

	// Try to read the string
	ret_value = fread(char_ptr, sizeof (char), strlen(str), rd_stream);
	if (ret_value != strlen(str))
	{
		printf("\nFread did not read %d elements as expected ", strlen(str));
		printf("\nret_value is %d ", ret_value);
		exit(6);
	}

	if (strncmp(char_ptr, str, strlen(str)))
	{
		printf("\ncontents of char_ptr are %s ", char_ptr);
		printf("\ncontents of str are %s ", str);
		printf("\nThese should be equal");
		exit(7);
	}

	ret_value = fclose(rd_stream);
	if (ret_value != 0)
	{
		printf("\nFclose failed for %s",fifoname);
		exit(8);
	}

	ret_value = remove(fifoname);
	if (ret_value != 0)
	{
		printf("\nremove failed for %s",fifoname);
		exit(9);
	}

	//pid = wait(c_status);
	if ((WIFEXITED(c_status) !=0) && (WEXITSTATUS(c_status) < 0))
	{
		printf("\nchild exited with code %d",WEXITSTATUS(c_status));
		exit(10);
    }

    printf("About to issue exit(0), processing completed successfully\n");
    exit(0);
}

static void createTokenRing()
{
    int pipe_fd;
    int res;
    int open_mode = O_RDONLY;
    char buffer[BUFFER_SIZE + 1];
    int bytes_read = 0;

    memset(buffer, '\0', sizeof(buffer));

    printf("Process %d opening FIFO O_RDONLY\n", getpid());
    pipe_fd = open(FIFO_NAME, open_mode);
    printf("Process %d result %d\n", getpid(), pipe_fd);

    if (pipe_fd != -1) {
        do {
            res = read(pipe_fd, buffer, BUFFER_SIZE);
            bytes_read += res;
        } while (res > 0);
        (void)close(pipe_fd);
    }
    else {
        exit(EXIT_FAILURE);
    }

    printf("Process %d finished, %d bytes read\n", getpid(), bytes_read);
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
 * Print message and exit
 * Input: msg, the message
 */
static inline void printAndDie(const char *msg)
{
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}
