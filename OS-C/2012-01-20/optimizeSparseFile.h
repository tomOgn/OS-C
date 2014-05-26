/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 gennaio 2012
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2012-01-20.pdf

@author: Tommaso Ognibene
*/

// Dependencies
#include "../const.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

static inline void errorAndDie(const char *msg);
static inline void printAndDie(const char *msg);
static void create_file_with_hole(void);
static void create_file_without_hole(void);

extern void run(int argc, char *argv[])
{
	create_file_with_hole();
	create_file_without_hole();
}

static void create_file_with_hole(void)
{
    int fd = open("file.hole", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    write(fd, "Hello", 5);
    lseek(fd, 99988, SEEK_CUR); // Make a hole
    write(fd, "Goodbye", 7);
    close(fd);
}

static void create_file_without_hole(void)
{
    int fd = open("file.nohole", O_WRONLY|O_TRUNC|O_CREAT, 0600);
    write(fd, "Hello", 5);
    char buf[99988];
    memset(buf, 'a', 99988);
    write(fd, buf, 99988); // Write lots of bytes
    write(fd, "Goodbye", 7);
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
