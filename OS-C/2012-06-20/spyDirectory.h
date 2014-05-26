/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012.06.20.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Function declarations
static inline int isDirectory(char *path);
static inline void printAndDie(const char *msg);
static inline void errorAndDie(const char *msg);
static void spyDirectory(char *directory);

// Constants
// Max number of events to process at one go
#define MAX_EVENTS 1024

// Max lenght of the filename
#define LEN_NAME 16

// Size of one event
#define EVENT_SIZE (sizeof (struct inotify_event))

// Buffer to store events
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME))

static void spyDirectory(char *directory)
{
	int fd, wd, length, i;
	char buffer[BUF_LEN];
	char **path = (char **) malloc(sizeof (char *) * 2);
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

	printf("Watching %s:\n", directory);

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

extern void run(int argc, char *argv[])
{
	if (argc != 2)
		printAndDie("The function requires one parameter to be passed in.");

	if (!isDirectory(argv[1]))
		printAndDie("The parameter should be an existing directory.");

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
