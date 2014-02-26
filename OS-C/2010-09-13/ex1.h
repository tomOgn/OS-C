#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static void setWriter(const char *path)
{
    int fd;

    // Create named pipe
    mkfifo(path, 0666);

    // Write output
    fd = open(path, O_WRONLY);
    write(fd, "Hi", sizeof("Hi"));
    close(fd);

    // Remove named pipe
    unlink(path);
}

extern void run(int argc, char *argv[])
{
	setWriter("/home/tommaso/Code/git/OS-C/OS-C/fifo");
}
