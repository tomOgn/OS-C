#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUF 1024

static void setReader(const char *path)
{
    int fd;
    char buf[MAX_BUF];

    // Create named pipe
    mkfifo(path, 0666);

    // Read output
    fd = open(path, O_RDONLY);
    read(fd, buf, MAX_BUF);

    printf("Received: %s\n", buf);

    close(fd);
}

extern void run(int argc, char *argv[])
{
	setReader("/home/tommaso/Code/git/OS-C/OS-C/fifo");
}
