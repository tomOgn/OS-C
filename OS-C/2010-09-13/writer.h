#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static inline void errorAndDie(const char *msg);

static void setWriter(const char *path)
{
    int fd;

    // Create named pipe
    mkfifo(path, 0666);

    // Write output
    fd = open(path, O_WRONLY);
    if (write(fd, "Hi", sizeof("Hi") < 0))
    	errorAndDie("write");

    close(fd);

    // Remove named pipe
    unlink(path);
}

extern void run(int argc, char *argv[])
{
	setWriter("/home/tommaso/Code/git/OS-C/OS-C/fifo");
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
