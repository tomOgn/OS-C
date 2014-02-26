// Dependencies

#include "../funRepository.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

#define _OPEN_SYS

static void run()
{
	int     flags, ret_value, c_status;
	pid_t   pid;
	size_t  n_elements;
	char    char_ptr[32];
	char    str[] = "string for fifo ";
	char    fifoname[] = "temp.fifo";
	FILE    *rd_stream,*wr_stream;

	if ((mkfifo(fifoname,S_IRWXU)) != 0)
		errorAndDie("mkfifo");

	if ((pid = fork()) < 0)
		errorAndDie("fork");

	if (pid == 0)
	{
		// CHILD process
		wr_stream = fopen(fifoname, "w");
		if (!wr_stream)
			errorAndDie("fopen");

		n_elements = fwrite(str,1, strlen(str), wr_stream);
		if (n_elements != strlen(str))
			errorAndDie("fwrite");

		exit(0);
	}
	else
	{
		// PARENT  process
		rd_stream = fopen(fifoname, "r");
		if (!rd_stream)
			errorAndDie("fopen");

		if ((flags = fcntl(fileno(rd_stream), F_GETFL)) == -1)
			errorAndDie("fcntl");

		flags &= (O_NONBLOCK);
		if ((fcntl(fileno(rd_stream), F_SETFL, flags)) == -1)
			errorAndDie("fcntl");

		ret_value = fread(char_ptr,sizeof(char),strlen(str),rd_stream);
		if (ret_value != strlen(str))
			errorAndDie("fread");

		if (strncmp(char_ptr, str, strlen(str)))
			errorAndDie("strncmp");

		ret_value = fclose(rd_stream);
		if (ret_value != 0)
			errorAndDie("fclose");

		ret_value = remove(fifoname);
		if (ret_value != 0)
			errorAndDie("remove");

		pid = wait(c_status);
		if ((WIFEXITED(c_status) !=0) && (WEXITSTATUS(c_status) !=0))
			errorAndDie("wait");
		}

		exit(0);
}
