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

/*
 * Run a specific process.
 * Wait until it terminates.
 * Input:	path,    the path to the file to run
 *          args,    the list of arguments
 * Output:	void
 */
static void runProcess(const char *path, char *args[])
{
	pid_t pid;

	// Create child process
	switch (pid = fork())
	{
	// Error
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	// Child process
	case 0:
		// Execute command
		if (execv(path, args) == -1)
		{
			perror("execv");
			exit(EXIT_FAILURE);
		}
		break;
	// Parent process
	}

	// Wait until the child process terminates
	if (wait(NULL) == -1)
	{
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/*
 * Check if a file is executable
 * Input:
 * 			path, the file file
 * Output:
 * 			1, if the is executable
 * 			0, else
 */
static inline int isExecutable(const char *path)
{
	struct stat *buf;
	return stat(path, &buf) == 0 && buf.st_mode && S_IXUSR;
}

static void runIfExecutable(const char *path, const char **args)
{
	if (isExecutable(path))
	{
		runProcess(path, args);

		if(remove(path) < 0)
		{
			perror("remove");
			exit(EXIT_FAILURE);
		}
	}
}

static int tryInotify(char *directory)
{
	int fd;		// file descriptor
	int	wd; 	// watch descriptor
	int	length;
	int	i = 0;
	char buffer[BUF_LEN];
	char **path = (char **) malloc(sizeof(char **) * 2);
	path[0] = directory;

	// Create an inotify instance
	fd = inotify_init();

	if (fd < 0)
	{
		perror("inotify_init");
		exit(EXIT_FAILURE);
	}

	// Create a watch list
	wd = inotify_add_watch(fd, directory, IN_CREATE);
	if (wd < 0)
	{
		perror("inotify_add_watch");
		exit(EXIT_FAILURE);
	}

	printf("Watching:: %s\n", directory);

	/* do it forever*/
	while (TRUE)
	{
		i = 0;
		length = read(fd, buffer, BUF_LEN);

		if (length < 0)
		{
			perror("read");
			exit(EXIT_FAILURE);
		}

		while (i < length)
		{
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len)
			{
				if (event->mask & IN_CREATE)
				{
					if (event->mask & IN_ISDIR)
					{
						printf("The directory %s was created.\n", event->name);
					}
					else
					{
						printf("The file %s was created.\n", event->name);

						path[1] = (char *) realloc(sizeof(char *) * len(event->name));
						strcpy(path[1], event->name);
						runIfExecutable(event->name);
					}
				}
			}
			i += EVENT_SIZE + event->len;
		}
	}

	// Clean up
	inotify_rm_watch(fd, wd);
	close(fd);

	exit(EXIT_SUCCESS);
}

extern void run(int argc, char *argv[])
{
	tryInotify("/home/tommaso/Downloads");
}
