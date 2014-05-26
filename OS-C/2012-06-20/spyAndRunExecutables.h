/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.06.20.pdf

@author: Tommaso Ognibene
*/

// Pre-processor directives
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Max number of events to process at one go
#define MAX_EVENTS 1024
// Max lenght of the filename
#define LEN_NAME 16
// Size of one event
#define EVENT_SIZE (sizeof (struct inotify_event))
// Buffer to store the events
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

// Function declarations
static inline int isDirectory(char *path);
static inline int isExecutable(char *path);
static inline void runIfExecutable(char *path, char *args[]);
static void spyDirectory(char *dir);
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);

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
 * Check if a file is executable. If yes, run it.
 * Input:  path, file path
 *         args, arguments list
 */
static inline void runIfExecutable(char *path, char *args[])
{
	pid_t pid;

	if (!isExecutable(path)) return;

	// Create child process
	if ((pid = fork()) < 0)
		errorAndDie("fork");

	// Child
	if (pid == 0)
	{
		// Execute command
		execvp(path, args);
		errorAndDie("execvp");
	}

	// Wait until the child process terminates
	if (wait(NULL) < 0)
		errorAndDie("waitpid");

	// Remove file
	if (remove(path) < 0)
		errorAndDie("remove");
}

static void spyDirectory(char *dir)
{
	int fd, wd, length, i, n;
	char buffer[BUF_LEN];
	char *path;
	char **args = (char **) malloc(sizeof(char **) * 2);
	struct inotify_event *event;

	i = 0;
	args[1] = NULL;

	// Create an inotify instance
	fd = inotify_init();
	if (fd < 0)
		errorAndDie("inotify_init");

	// Create a watch list
	wd = inotify_add_watch(fd, dir, IN_CREATE);
	if (wd < 0)
		errorAndDie("inotify_add_watch");

	printf("Watching %s\n", dir);

	while (True)
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
					{
						printf("The file %s was created.\n", event->name);

						n = strlen(dir) + strlen(event->name) + 1;
						path = (char *) malloc(n * sizeof(char *));
						sprintf(path, "%s/%s", dir, event->name);
						args[0] = event->name;
						runIfExecutable(path, args);
					}
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}

	// Clean up
	inotify_rm_watch(fd, wd);
	close(fd);
}

extern void run(int argc, char *argv[])
{
	if (argc != 2)
		printAndDie("The function requires only one parameter to be passed in.\n");

	if (!isDirectory(argv[1]))
		printAndDie("The parameter should be an existing directory.\n");

	spyDirectory(argv[1]);
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
