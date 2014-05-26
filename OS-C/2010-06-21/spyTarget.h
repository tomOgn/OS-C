/*
Prova Pratica di Laboratorio di Sistemi Operativi
21 giugno 2010
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2010-06-21.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/inotify.h>

// Constants
#define True            1
#define False           0
#define Success         True
#define Failure         False
#define MaxEvents       1024
#define MaxLenArr       128
#define MaxLenStr       256
#define EventSize       sizeof (struct inotify_event)
#define EventBufferSize 1024 * (EventSize + 16)

// Function declarations
static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void spyTarget(char *target);
static inline int isFile(char *path);

extern void run(int argc, char *argv[])
{
	// Pre-conditions
	if (argc != 2)
		printAndDie("The function requires one parameter to be passed in.");

	if (!isFile(argv[1]))
		printAndDie("The parameter should be an existing file or directory.");

	// Spy target
	spyTarget(argv[1]);
}

/*
 * Spy a specific file or directory.
 * Check when it is opened and changed.
 * Stop when it is deleted.
 * Input: target, the file or directory
 */
static void spyTarget(char *target)
{
	int fd, wd, length, i, deleted;
	char buffer[EventBufferSize];
	struct inotify_event *event;

	// Initialize inotify instance
	fd = inotify_init();
	if (fd < 0)
		errorAndDie("inotify_init");

	// Create a watch list
	wd = inotify_add_watch(fd, target, IN_OPEN | IN_ACCESS | IN_MODIFY | IN_DELETE);
	if (wd < 0)
		errorAndDie("inotify_add_watch");

	printf("Watching %s...\n", target);

	// Loop until the file is deleted
	deleted = False;
	while (!deleted)
	{
		// Get events
		length = read(fd, buffer, EventBufferSize);
		if (length < 0)
			errorAndDie("read");

		// Analyze events
		for (i = 0; i < length; i += EventSize + event->len)
		{
			event = (struct inotify_event *) &buffer[i];
			if (event->len)
			{
				// Open event
				if (event->mask & IN_OPEN)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s has been opened.\n", event->name);
					else
						printf("The file %s has been opened.\n", event->name);
				}
				// Access event
				else if (event->mask & IN_ACCESS)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s has been accessed.\n", event->name);
					else
						printf("The file %s has been accessed.\n", event->name);
				}
				// Modify event
				else if (event->mask & IN_MODIFY)
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s has been modified.\n", event->name);
					else
						printf("The file %s has been modified.\n", event->name);
				}
				// Delete event
				else
				{
					if (event->mask & IN_ISDIR)
						printf("The directory %s has been deleted.\n", event->name);
					else
						printf("The file %s has been deleted.\n", event->name);

					deleted = True;
				}
			}
		}
	}

	// Garbage collection
	inotify_rm_watch(fd, wd);
	close(fd);
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
