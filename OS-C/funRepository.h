// FUN-ctions Repository
#ifndef FUNREPO_H
#define FUNREPO_H

// Dependencies
#include "../const.h"

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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Constants
// Max number of events to process at one go
#define MAX_EVENTS 1024
// Max lenght of the filename
#define LEN_NAME 16
// Size of one event
#define EVENT_SIZE (sizeof (struct inotify_event))
// Buffer to store the events
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))
#define MaxLenArr 128 // Max length array
#define MaxLenStr 256 // Max length string
#define BUFFER_SIZE 1024

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

// Function declarations
static void compareDirectories(char *dirA, char *dirB);
static inline void errorAndDie(const char *msg);
int filterExtensions(const struct dirent *entry);
int filterNames(const struct dirent *entry);
static inline char *getAbsolutePath(char *dirPath, char *filePath);
static inline void getCurrentProcesses(void);
static int getLenghtOfLine(char *path, int theLine, int *lenght);
static inline int getSizeFile(char *path, off_t *size);
static inline int getSoftLink(char *filePath, char **softLink);
static int hasExtension(const char *fileName, const char **extensions, int n);
int iNodeComparison(const struct dirent **a, const struct dirent **b);
static inline void inverseArray(int x[], int n);
static inline int isDirectory(char *path);
static inline int isExecutable(char *path);
static inline int isFile(char *path);
static inline int isHiddenFile(char *path);
static inline int isInteger(char *n);
static inline int isNatural(int n);
static inline int isNumber(char *text);
static inline int isProcess(int pid);
static inline int isSignalNumber(int n);
static inline int isTextFile(char *path);
static inline int max(int a[], int n);
static inline Process *parseArguments(int argc, char **argv);
static inline void pollNamedPipes(char *name[], int n);
static inline void printArray(int x[], int n);
static inline void printAndDie(const char *msg);
static inline void readPipe(int fd);
static inline void runProcess(char *path, char *command[]);
static inline void runProcessN(Process *process, int n);
static inline int sameContent(FILE *f1, FILE *f2);
static void scanDirectory(char *dir);
static inline void simplePipe(void);
static inline void signalRepeater(int pid);
static void spyDirectory(char *directory);
static inline void writePipe(int fd);

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
	char buffer[BUFFER_SIZE];

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
 * Output:  1,        if success
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
	return entry->d_type == DT_DIR && isNumber(entry->d_name);
}

/*
 * Check whether a string is made only by numbers or not
 * Input:   text, the string
 * Output:  1,    if the string is only made by numbers
 * 			0,    else
 */
static inline int isNumber(char *text)
{
	regex_t regExp;
	char *pattern = "[0-9]";
	int status;

    if (regcomp(&regExp, pattern, REG_EXTENDED) != 0)
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

	while (TRUE)
	{
		if (sigwait(&sigSet, &sigNum) < 0)
			errorAndDie("sigwait");

		kill(pid, sigNum);
	}
}

/*
 * Check whether a PID refers to a running process or not.
 * Input:  pid,   the PID to check
 * Output: TRUE,  if yes
 *         FALSE, otherwise
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

	for(i = 0; i < n; i++)
		printf("%d ", x[i]);
	printf("\n");
}

/*
 * Invert the elements of an array.
 * Input:  x, array
 *         n, array size
 */
static inline void inverseArray(int x[], int n)
{
	int i, tmp;

	n--;
	for(i = 0; i < n / 2; i++)
	{
		tmp = x[i];
		x[i] = x[n];
		x[n] = tmp;
		n--;
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
	int output = FALSE;
	int i = 0;

	if (extension)
		while (i < n && !output)
		{
			if (!strcmp(extension, extensions[i]))
				output = TRUE;
			else
				i++;
		}

	return output;
}

/*
 * Check, byte by byte, if two files have same content.
 * Input:   f1,    pointer to file 1
 * 		    f2,    pointer to file 2
 * Output:  TRUE,  if they have the same content
 * 			FALSE, else
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

static void spyDirectory(char *directory)
{
	int fd, wd, length, i;
	char buffer[BUF_LEN];
	char **path = (char **) malloc(sizeof(char **) * 2);
	struct inotify_event *event;

	path[0] = directory;
	i = 0;

	// Create an inotify instance
	fd = inotify_init();

	if (fd < 0)
		errorAndDie("inotify_init");

	// Create a watch list
	wd = inotify_add_watch(fd, directory, IN_CREATE);
	if (wd < 0)
		errorAndDie("inotify_add_watch");

	printf("Watching %s\n", directory);

	while (TRUE)
	{
		i = 0;
		length = read(fd, buffer, BUF_LEN);

		if (length < 0)
			errorAndDie("read");

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
			i += EVENT_SIZE + event->len;
		}
	}

	// Clean up
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
		output = TRUE;
	}
	else
	{
		*lenght = 0;
		output = FALSE;
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
 * If yes, return its size in Bytes.
 * Input:  path, file path
 * Output: 1,    if it is a regular file
 * 		   0,    else
 *
 * 		   size, its size
 */
static inline int getSizeFile(char *path, off_t *size)
{
	struct stat info;
	int output = FALSE;

	if (stat(path, &info) < 0)
		errorAndDie("stat");

	if (info.st_mode & S_IFREG)
	{
		output = TRUE;
		*size = info.st_size;
	}

	return output;
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
 * Check if a file is regular
 * Input:  path, file path
 * Output: 1,    if the file is a regular file
 * 		   0,    else
 */
static inline int isFile(char *path)
{
	struct stat info;

	// Get file attributes
	if (!lstat(path, &info) < 0)
		errorAndDie("lstat");

	return S_ISREG(info.st_mode);
}

#endif
