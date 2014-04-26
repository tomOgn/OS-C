// FUN-ctions Repository
#ifndef FUNREPO_H
#define FUNREPO_H

// Dependencies
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Constants
#define KB         1024
#define True       1
#define False      0
#define Success    True
#define Failure    False
#define MaxEvents  1024
#define EventSize  (sizeof (struct inotify_event))
#define MaxLenArr  128
#define MaxLenStr  256
#define BufferSize 1024

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

typedef struct
{
	int n;
} sharedRegion;

// Function declarations
static inline int      areEqual(int x[], int y[], int n);
static int             compareAlphabetic(const void *s1, const void *s2);
static void            compareDirectories(char *dirA, char *dirB);
static inline void     errorAndDie(const char *msg);
int                    filterExtensions(const struct dirent *entry);
int                    filterNames(const struct dirent *entry);
static inline char *   getAbsolutePath(char *dirPath, char *filePath);
static inline void     getCurrentProcesses(void);
static int             getLenghtOfLine(char *path, int theLine, int *lenght);
static inline int      getSizeFile(char *path, off_t *size);
static inline int      getSoftLink(char *filePath, char **softLink);
static int             hasExtension(const char *fileName, const char **extensions, int n);
int                    iNodeComparison(const struct dirent **a, const struct dirent **b);
static inline void     inverseArray(int x[], int n);
static inline int      inverseFile(char *pathSrc, char *pathDst, off_t chunk);
static inline int      isDirectory(char *path);
static inline int      isExecutable(char *path);
static inline int      isFile(char *path);
static inline int      isHiddenFile(char *path);
static inline int      isInteger(char *n);
static inline int      isNatural(int n);
static inline int      isNumberR(char *text);
static inline int      isNumber(char *str);
static inline int      isProcess(int pid);
static inline int      isSignalNumber(int n);
static inline int      isTextFile(char *path);
static inline int      max(int a[], int n);
static inline void     outputNamedPipe(char *name[], int n);
static inline Process *parseArguments(int argc, char **argv);
static inline void     pollNamedPipes(char *name[], int n);
static inline void     printArray(int x[], int n);
static inline void     printArrayR(char **x, int n);
static inline void     printAndDie(const char *msg);
static inline void     printDirectory(char *path);
static inline void     readPipe(int fd);
static inline void     redirectOutputN(int n);
static inline void     runProcess(char *path, char *command[]);
static inline void     runProcessN(Process *process, int n);
static inline int      sameContent(FILE *f1, FILE *f2);
static void            scanDirectory(char *dir);
static inline void     signalRepeater(int pid);
static inline void     simplePipe(void);
static void            spyDirectory(char *directory);
static inline void     testPing(char *message, int times);
static inline void     testTokenRing(int n);
static inline void     writePipe(int fd);

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
	memcpy((void *) &tmp, (void *) &set, sizeof(fd_set));

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

/*char buffer[Lenght];
 * Run n copies of a process.
 * Redirect standard output from children to the parent.
 * Input: n, number of children
 */
static inline void redirectOutputN(int n)
{
	char buf[4096];
	int started, terminated;
	int **fd;
	pid_t pid;
	ssize_t count;

	// Initialize file descriptors to be used for the pipeline
	fd = (int **) malloc(sizeof (int *) * n);
	for (started = 0; started < n; started++)
		fd[started] = (int *) malloc (sizeof (int) * 2);

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
			printf("Hello from %d\n", getpid());

			exit(EXIT_SUCCESS);
		}

		// Close output side of pipe
		close(fd[started][1]);

		// Read on the pipe
		while ((count = read(fd[started][0], buf, sizeof (buf))) > 0)
		{
			if (write(1, buf, count) < 0)
				errorAndDie("write");

			memset(buf, 0, sizeof (buf));
		}
		if (count < 0)
			errorAndDie("read");
	}

	// Wait processes
	for (terminated = 0; terminated < started; terminated++)
		if (wait(NULL) < 0)
			errorAndDie("wait");
}

/*
 * Inizialize a shared memory object.
 * Input:  name, shared memory object' name
 * Output: pointer to the shared memory object
 * Require: sys/mman.h
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

	// Establish a mapping between process address space and shared memory
	ptr = mmap(0, sizeof (sharedRegion), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		errorAndDie("mmap");

	return ptr;
}

/*
 * Comparison function to sort alphabetically.
 * Input:  s1, sentence 1
 *         s2, sentence 2
 * Output: 1,  if sentence 1 has a greater value than sentence 2
 *         0,  else
 */
static int compareAlphabetic(const void *s1, const void *s2)
{
    const char **i1 = (const char **)s1;
    const char **i2 = (const char **)s2;

    return strcmp(*i1, *i2);
}

/*
 * Print directory entries in reverse alphabetic order.
 * Input: path, path to the directory
 */
static inline void printDirectory(char *path)
{
	DIR *root;
	struct dirent *entry;
	char **file;
	int n, size;

	// Open directory
	root = opendir(path);
	if (!root)
		errorAndDie("opendir");

	// Initialize variables
	n = -1;
	size = 100 * sizeof (char *);
	file = (char **) malloc(size);
	if (!file)
		errorAndDie("malloc");

	// Get directory entries
	entry = readdir(root);
	while (entry)
	{
		// Adapt the array size to the number of entries
		n++;
		if (n == size)
		{
			size *= 2;
			file = (char **) realloc(file, size);
			if (!file)
				errorAndDie("realloc");
		}

		// Get the entry name
		file[n] = (char *) malloc(sizeof (char) * MaxLenStr);
		if (!file[n])
			errorAndDie("malloc");

		memset(file[n], 0, sizeof (char) * MaxLenStr);
		strcpy(file[n], entry->d_name);

		entry = readdir(root);
	}

	if (n == 0)
		printAndDie("The directory is empty.");

	// Sort alphabetically
	qsort(file, n, sizeof (char *), compareAlphabetic);

	// Print in reverse order
	printArrayR(file, n);
}

/*
 * Inverse a file chunk by chunk.
 * Input:  pathSrc, path to the source file
 *         pathDst, path to the destination file
 *         chunk,   chunk of bytes
 * Output: 1, Success
 *         0, Failure
 */
static inline int inverseFile(char *pathSrc, char *pathDst, off_t chunk)
{
	off_t offset, size;
	char *buffer;
	int src, dst;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // Full access

	// Pre-conditions: source exists, destination does not
	if (!getSizeFile(pathSrc, &size) || isFile(pathDst))
		return Failure;

	// Open source file
	src = open(pathSrc, O_RDONLY);
	if (src < 0)
		errorAndDie("open");

	// Create destination file
	dst = creat(pathDst, mode);
	if (dst < 0)
		errorAndDie("creat");

	// Set size of destination
	if (ftruncate(dst, size) < 0)
		errorAndDie("ftruncate");

	// Define the content buffer
	buffer = (char *) malloc(chunk * sizeof (char));

	// Inverse the file chunk by chunk
	for (offset = 0; offset + chunk <= size; offset += chunk)
	{
		// Clear buffer
		memset(buffer, 0, chunk);

		// Read from source
		if (pread(src, buffer, chunk, offset) < 0)
			errorAndDie("pread");

		// Inverse data
		inverseArray(buffer, chunk);

		// Write to destination
		if (pwrite(dst, buffer, chunk, size - offset - chunk) < 0)
			errorAndDie("pwrite");
	}

	// Check if there is a last minor chunk to process
	if (offset != size)
	{
		// Clear buffer
		memset(buffer, 0, chunk);

		// Read from source
		if (pread(src, buffer, size - offset, offset) < 0)
			errorAndDie("pread");

		// Inverse data
		inverseArray(buffer, size - offset);

		// Write to destination
		if (pwrite(dst, buffer, size - offset, 0) < 0)
			errorAndDie("pwrite");
	}

	// Close file descriptors
	if (close(src) < 0 || close(dst) < 0)
		errorAndDie("close");

    // Garbage collection
	free(buffer);

	return Success;
}

/*
 * Check if two array have the same elements.
 * Input:  x, array 1
 *         y, array 2
 *         n, array size
 * Output: 1, if they are equal
 *         0, else
 */
static inline int areEqual(int x[], int y[], int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (x[i] != y[i])
			return False;

	return True;
}

/*
 * Ping Test
 * Create a child process. Send a message n times and check the reply.
 * Count the overall elapsed time.
 * Input: message, the message to send
 *        times,   the n times
 */
static inline void testPing(char *message, int times)
{
	clock_t start, end;
	double elapsed;
	pid_t pid;
	int i;
	int fd[2][2];
	const int ToChild = 0, FromChild = 1, Lenght = strlen(message) + 1;
	char buffer[Lenght];

	// Create two pipes
	if (pipe(fd[ToChild]) < 0)
		errorAndDie("pipe");

	if (pipe(fd[FromChild]) < 0)
		errorAndDie("pipe");

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		// Close input side of pipe FromChild
		close(fd[FromChild][0]);

		// Close output side of pipe ToChild
		close(fd[ToChild][1]);

		while (1)
		{
			// Read on the pipe ToChild
			if (read(fd[ToChild][0], buffer, Lenght) < 0)
				errorAndDie("read");

			// Write on the pipe FromChild
			if (write(fd[FromChild][1], buffer, Lenght) < 0)
				errorAndDie("write");
		}
	}

	// Close input side of pipe ToChild
	close(fd[ToChild][0]);

	// Close output side of pipe FromChild
	close(fd[FromChild][1]);

	// Start time
	start = clock();

	// Ping Test
	for (i = 0; i < times; i++)
	{
		// Write on the pipe ToChild
		if (write(fd[ToChild][1], message, Lenght) < 0)
			errorAndDie("write");

		// Read on the pipe FromChild
		if (read(fd[FromChild][0], buffer, Lenght) < 0)
			errorAndDie("read");

		if (strcmp(message, buffer))
			printAndDie("Ping Test failed!\nThe child process has sent back a wrong message.");
	}

	// Stop time
	end = clock();

	// Kill child process
	kill(pid, SIGTERM);

	// Print elapsed time
	elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("Ping Test succeeded!\n");
	printf("Test Message = %s\n", message);
	printf("Repetitions  = %d\n", times);
	printf("Elapsed Time = %0.5f\n", elapsed);
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
				// Fill the buffer
				count = read(fds[i].fd, &buffer, sizeof (buffer));
				if (count < 0)
					errorAndDie("read");

				// Deliver the buffer to the standard output
				if (write(STDOUT_FILENO, buffer, count) < 0)
					errorAndDie("write");

				// Clear the buffer
				memset(buffer, 0, sizeof (buffer));
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
 * Run a process.
 * Wait until it terminates.
 * Input: path,    file to run
 *        command, list of arguments
 */
static inline void runProcess(char *path, char *command[])
{
	pid_t pid;

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("fork");

	// Child process
	if (pid == 0)
	{
		// Execute command
		if (execvp(path, command) < 0)
			errorAndDie("execve");

		exit(EXIT_SUCCESS);
	}

	// Wait until it terminates
	if (wait(NULL) < 0)
		errorAndDie("wait");
}

/*
 * Get current processes
 * Input:   None
 * Output:  Print PID and command for each running process
 */
static inline void getCurrentProcesses(void)
{
	int count, i;
	struct dirent **entries;
	char process[1024];
	char *softLink, *PATH;

	softLink = NULL;
	PATH = "/proc";
	count = scandir(PATH, &entries, filterNames, NULL);

	// Check if any errors occurred
	if (count < 0)
		errorAndDie("scandir");

	for (i = 0; i < count; i++)
	{
		snprintf(process, sizeof process, "%s%s%s%s", PATH, "/", entries[i]->d_name, "/exe");
		if (getSoftLink(process, &softLink))
			printf("%s %s\n", entries[i]->d_name, softLink);
	}
}

/*
 * Get a soft link from a given file path
 * Input:   filePath, the file path
 * Output:  1,        if Success
 * 			0,        else
 *
 * 			softLink, the soft link
 */
static inline int getSoftLink(char *filePath, char **softLink)
{
	int size, count;

	size = 64;
	do
	{
		*softLink = (char *) realloc(*softLink, size);
		count = readlink(filePath, *softLink, size);
		size *= 2;
	}
	while (count > size);

	return count > 0;
}

/*
 * Filter function for directory entries
 * Input:   entry, directory entry
 * Output:  1,     if the entry is a directory and its name is only made by numbers
 * 			0,     else
 */
int filterNames(const struct dirent *entry)
{
	return entry->d_type == DT_DIR && isNumberR(entry->d_name);
}

/*
 * Check whether a string is made only by numbers or not
 * Input:   text, the string
 * Output:  1,    if the string is only made by numbers
 * 			0,    else
 */
static inline int isNumberR(char *text)
{
	regex_t regExp;
	char *pattern = "[0-9]";
	int status;

    if (regcomp(&regExp, pattern, REG_EXTENDED) < 0)
    	errorAndDie("regcomp");

    status = regexec(&regExp, text, regExp.re_nsub, NULL, 0);
    regfree(&regExp);

    return !status;
}

/*
 * Check if a file is executable
 * Input:  path, file path
 * Output: 1,    if it is executable
 * 		   0,    else
 */
static inline int isExecutable(char *path)
{
	return access(path, X_OK) == 0;
}

/*
 * Parse command line arguments.
 * Input: argc, argument counter
 *        argv, argument vector
 * Output:
 *        Process, data structure containing:
 *           pth, file to run
 *           cmd, list of arguments
 *           env, environment variable
 */
static inline Process *parseArguments(int argc, char **argv)
{
	int i;
	Process *process = (Process *) malloc(sizeof (Process));

	// Set the file path
	process->pth = argv[2];

	// Set the argument list
	for (i = 0; i < argc - 2; i++)
		process->cmd[i] = argv[i + 2];
	process->cmd[i] = NULL;

	// Initialize the environment variable
	process->env[0] = (char *) malloc(sizeof (char) * MaxLenStr);
	process->env[1] = NULL;

	return process;
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
static inline void runProcessN(Process *process, int n)
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

/*
 * Read data from pipe to standard output.
 * Input: fd, file descriptor
 */
static inline void readPipe(int fd)
{
	FILE *stream;
	int c;

	stream = fdopen(fd, "r");
	while ((c = fgetc(stream)) != EOF)
		putchar(c);
	fclose(stream);
}

/*
 * Write data to pipe.
 * Input: fd, file descriptor
 */
static inline void writePipe(int fd)
{
	FILE *stream;

	stream = fdopen(fd, "w");
	fprintf(stream, "hello world!\n");
	fclose(stream);
}

/*
 * Basic input/output from/to pipe.
 */
static inline void simplePipe(void)
{
	pid_t pid;
	int mypipe[2];

	// Create pipe
	if (pipe(mypipe) < 0)
		errorAndDie("pipe");

	// Fork
	pid = fork();
	if (pid < 0)
		errorAndDie("pipe");

	// Child process
	if (pid == 0)
	{
		// Close input side of pipe
		close(mypipe[0]);

		// Write to pipe
		writePipe(mypipe[1]);

		exit(EXIT_SUCCESS);
	}

	// Close output side of pipe
	close(mypipe[1]);

	// Read from pipe
	readPipe(mypipe[0]);
}

/*
 * Send signals to a running process.
 * Input:  pid,   the running process
 */
static inline void signalRepeater(int pid)
{
	int sigNum, i;
	sigset_t sigSet;
	int *terminatingSignals = { SIGTERM, SIGINT, SIGQUIT, SIGKILL, SIGHUP };

	// Initialize and empty a signal set
	if (sigemptyset(&sigSet) < 0)
		errorAndDie("sigemptyset");

	// Initialize a signal set to full, including all signals
	if (sigfillset(&sigSet) < 0)
		errorAndDie("sigfillset");

	// Remove terminating signals
	for (i = 0; i < 5; i++)
		if (sigdelset(&sigSet, terminatingSignals[i++]) < 0)
			errorAndDie("sigdelset");

	while (True)
	{
		if (sigwait(&sigSet, &sigNum) < 0)
			errorAndDie("sigwait");

		kill(pid, sigNum);
	}
}

/*
 * Check whether a PID refers to a running process or not.
 * Input:  pid,   the PID to check
 * Output: True,  if yes
 *         False, otherwise
 */
static inline int isProcess(int pid)
{
	return !kill(pid, 0);
}

/*
 * Find the max in an array.
 * Input:  a, an array
 *         n, its size
 * Output: the maximum value
 */
static inline int max(int a[], int n)
{
	int i, max = a[0];

	for (i = 1; i < n; i++)
		if (a[i] > max)
			max = a[i];

	return max;
}

/*
 * Print the elements of an array.
 * Input:  x, array
 *         n, array size
 */
static inline void printArray(int x[], int n)
{
	int i;

	for (i = 0; i < n; i++)
		printf("%d ", x[i]);
	printf("\n");
}

/*
 * Print the elements of an array in reverse order.
 * Input:  x, array
 *         n, array size
 */
static inline void printArrayR(char **x, int n)
{
	int i;

	for (i = n - 1; i >= 0; i--)
		printf("%s\n", x[i]);
	printf("\n");
}

/*
 * Invert the elements of an array. In-place, with swapping.
 * Input:  x, array
 *         n, array size
 */
static inline void inverseArray(char *x, int n)
{
	int left, right, temp;

	right = n - 1;
	for (left = 0; left < n / 2; left++)
	{
		temp = x[left];
		x[left] = x[right];
		x[right] = temp;
		right--;
	}
}

/*
 * Check if a number is natural.
 * Input:  n, the number
 * Output: 1, if the number is natural
 * 		   0, else
 */
static inline int isNatural(int n)
{
	return n >= 0 && n == floor(n);
}

/*
 * Check whether a string represents an integer or not.
 * Input:  n, the string
 * Output: 1, if the string represents an integer
 * 		   0, else
 */
static inline int isInteger(char *n)
{
	long int result = strtol(n, NULL, 10);

	return result != LONG_MIN && result != LONG_MAX;
}

/*
 * Check if a number is within the range of supported real-time signals.
 * Input:  n, number
 * Output: 1, if the number is within the range
 * 		   0, else
 */
static inline int isSignalNumber(int n)
{
	return isNatural(n) && n >= 0 && n <= 64;
}

/*
 * Scan two directories. Order the entries alphabetically.
 * Print if a file is in dirA but not in dirB and vice versa.
 * Input: dirA & dirB, path to directories A and B
 */
static void compareDirectories(char *dirA, char *dirB)
{
	int result, countA, countB, itA, itB;
	struct dirent **filesA, **filesB;

	countA = scandir(dirA, &filesA, filterExtensions, alphasort);
	countB = scandir(dirB, &filesB, filterExtensions, alphasort);
	if (countA < 0 || countB < 0)
		errorAndDie("scandir");

	// Loop through directory entries
	itA = itB = 0;
	while (itA < countA && itB < countB)
	{
		result = strcmp(filesA[itA]->d_name, filesB[itB]->d_name);
		if (result > 0)
		{
			printf("%s not in %s\n", filesB[itB]->d_name, dirA);
			itB++;
		}
		else if (result < 0)
		{
			printf("%s not in %s\n", filesA[itA]->d_name, dirB);
			itA++;
		}
		else
		{
			itA++;
			itB++;
		}
	}
	if (itA < countA)
		for (; itA < countA; itA++)
			printf("%s not in %s\n", filesA[itA]->d_name, dirB);
	else
		for (; itB < countB; itB++)
			printf("%s not in %s\n", filesB[itB]->d_name, dirA);
}

/*
 * Filter function for directories
 * Input:  entry, pointer to the directory entry
 * Output: 1,     if the entry has .h or .c extension
 * 		   0,     else
 */
int filterExtensions(const struct dirent *entry)
{
	const char *exts[] = { "c", "h"};

	return entry->d_type == DT_REG && hasExtension(entry->d_name, exts, 2);
}

/*
 * Check if a file name has a particular extension.
 * Input:   fileName, 	file name
 * 			extensions,	array of possible extensions
 * 			n,			dimension of the array
 *
 * Output:
 * 			1,          if the entry has one of the possible extensions
 * 			0,          else
 */
static int hasExtension(const char *fileName, const char **extensions, int n)
{
	char *extension = strrchr(fileName, '.') + 1;
	int output = False;
	int i = 0;

	if (extension)
		while (i < n && !output)
		{
			if (!strcmp(extension, extensions[i]))
				output = True;
			else
				i++;
		}

	return output;
}

/*
 * Check, byte by byte, if two files have same content.
 * Input:   f1,    pointer to file 1
 * 		    f2,    pointer to file 2
 * Output:  True,  if they have the same content
 * 			False, else
 */
static inline int sameContent(FILE *f1, FILE *f2)
{
	while (!feof(f1) && fgetc(f1) == fgetc(f2));

	return feof(f1) && feof(f2);
}

/*
 * Get the full path of a file.
 * Input:  dirPath,  directory path
 * 		   fileName, file name
 * Output: pointer to the file path
 */
static inline char *getAbsolutePath(char *dirPath, char *fileName)
{
	int lenght = strlen(dirPath) + strlen(fileName) + 1;
	char *filePath = (char *) malloc(lenght * sizeof(char *));
	if (!filePath)
		errorAndDie("malloc");

	sprintf(filePath, "%s/%s", dirPath, fileName);

	return filePath;
}

/*
 * Comparison function for iNode
 * Input:   a & b,  pointers to directory entries
 * Output:  1,	    if iNode[a] >  iNode[b]
 * 			0,	    if iNode[a] == iNode[b]
 * 		   -1,      if iNode[a] <  iNode[b]
 */
int iNodeComparison(const struct dirent **a, const struct dirent **b)
{
    long iNodeA, iNodeB;

    iNodeA = (long)(*a)->d_ino;
    iNodeB = (long)(*b)->d_ino;

    return (iNodeA > iNodeB) - (iNodeA < iNodeB);
}

/*
 * Scan a directory. Order entries by iNode number.
 * Print name and iNode of each file.
 * Input: dir, the directory
 */
void scanDirectory(char *dir)
{
	int result, it;
	struct dirent **files;

	result = scandir(dir, &files, NULL, iNodeComparison);

	if (result < 0)
		errorAndDie("scandir");

	// Loop through directory entries
	for (it = 0; it < result; it++)
	{
		// Do the printing stuff
		printf("%s/%s %li\n", dir, files[it]->d_name, (long)files[it]->d_ino);

		// Garbage collection
		free(files[it]);
	}
	free(files);
}

/*
 * Spy a directory. Check when a file is created, changed and deleted.
 * Input: directory, the directory
 */
static void spyDirectory(char *directory)
{
	int fd, wd, length, i;
	char buffer[BufferSize];
	struct inotify_event *event;

	// Create an inotify instance
	fd = inotify_init();
	if (fd < 0)
		errorAndDie("inotify_init");

	// Create a watch list
	wd = inotify_add_watch(fd, directory, IN_CREATE);
	if (wd < 0)
		errorAndDie("inotify_add_watch");

	printf("Watching %s...\n", directory);

	while (True)
	{
		length = read(fd, buffer, BufferSize);
		if (length < 0)
			errorAndDie("read");

		i = 0;
		while (i < length)
		{
			event = (struct inotify_event *) &buffer[i];
			if (event->len)
			{
				if (event->mask & IN_CREATE)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s was created.\n", event->name);
					else
						printf("The file %s was created.\n", event->name);
				}
			}
			i += EventSize + event->len;
		}
	}

	// Garbage collection
	inotify_rm_watch(fd, wd);
	close(fd);
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

/*
 * Find a specific line in a text file and get its lenght (i.e. number of characters).
 * Inpstatic inline int isDirectory(char *path);ut:
 * 			path,       file path
 * 			theLine,    line number
 * Output:
 * 			lenght,     lenght of the line
 *
 * 			1,          in case of success
 * 			0,          else
 */
static int getLenghtOfLine(char *path, int theLine, int *lenght)
{
	int c, count, line, output;
	FILE *stream;

	count = 0;
	line = 0;
	stream = fopen(path, "r");
	if (!stream)
		errorAndDie("fopen");

	while ((c = fgetc(stream)) != EOF && line < theLine)
	{
		if (c == '\n')
		{
			line++;
			if (line < theLine)
				count = 0;
		}
		else
			count++;
	}

	fclose(stream);

	if (line == theLine)
	{
		*lenght = count;
		output = True;
	}
	else
	{
		*lenght = 0;
		output = False;
	}

	return output;
}

/*
 * Check if a file is a directory
 * Input:  path, file path
 * Output: 1,    if the file is a directory
 * 		   0,    else
 */
static inline int isDirectory(char *path)
{
	struct stat info;

	if (stat(path, &info) < 0)
		errorAndDie("stat");

	return info.st_mode & S_IFDIR;
}

/*
 * Check if a file is a regular file.
 * If yes, return its size in bytes.
 * Input:  path, file path
 * Output: 1,    if it is a regular file
 * 		   0,    else
 *
 * 		   size, its size
 */
static inline int getSizeFile(char *path, off_t *size)
{
	struct stat info;

	if (isFile(path))
	{
		if (stat(path, &info) < 0)
			errorAndDie("stat");

		if (info.st_mode & S_IFREG)
		{
			*size = info.st_size;
			return True;
		}
	}

	return False;
}

/*
 * Check if a file is textual
 * Input:  path, file pointer
 * Output: 1,    if the file is a text file
 * 		   0,    else
 */
static inline int isTextFile(char *path)
{
	int c;
	FILE *fp = fopen(path, "r");

	if (!fp)
		errorAndDie("fopen");

	while ((c = fgetc(fp)) != EOF && c >= 1 && c <= 127);
	fclose(fp);

	return c == EOF;
}

/*
 * Check if a file is hidden
 * Input:  path, file path
 * Output: 1,    if the file is an hidden file
 * 		   0,    else
 */
static inline int isHiddenFile(char *path)
{
	FILE *fp = fopen(path, "r");
	if (!fp)
		errorAndDie("fopen");

	return fgetc(fp) == '.';
}

/*
 * Check if a file exists.
 * Input:  path, file path
 * Output: 1,    if the file exists
 * 		   0,    else
 */
static inline int isFile(char *path)
{
	FILE *file = fopen(path, "r");

	if (file)
	{
		fclose(file);
		return True;
	}

	return False;
}

#endif
