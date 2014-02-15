/*
Prova Pratica di Laboratorio di Sistemi Operativi
20 giugno 2013
Esercizio 1

URL: http://www.cs.unibo.it/~renzo/so/pratiche/2013.05.29.pdf

@author: Tommaso Ognibene
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <regex.h>
#include <unistd.h>
#include <string.h>

char *PATH = "/proc";

/*
 * Check whether a string is made only by numbers or not
 * Input:
 * 			1 string
 * Output:
 * 			1, if the string is only made by numbers
 * 			0, else
 */
int isOnlyMadeByNumbers(const char *name)
{
	regex_t regExp;
	char *pattern = "[0-9]";
	int status;

    if(regcomp(&regExp, pattern, REG_EXTENDED) != 0)
    {
		perror("regcomp");
		exit(EXIT_FAILURE);
    }

    status = regexec(&regExp, name, regExp.re_nsub, NULL, 0);
    regfree(&regExp);

    return !status;
}

/*
 * Filter function for Directories
 * Input:
 * 			1 dirent pointer
 * Output:
 * 			1, if the entry is a directory and its name is only made by numbers
 * 			0, else
 */
int filterDirectories(const struct dirent *entry)
{
	return entry->d_type == DT_DIR && isOnlyMadeByNumbers(entry->d_name);
}

/*
 * Get a soft link from a given file path
 * Input:
 * 			1 string,
 * Output:
 * 			1, if success
 * 			0, else
 *
 * 			the soft link
 */
int getSoftLink(const char *filePath, char **softLink)
{
	int size = 50, count;

	do
	{
		size *= 2;
		*softLink = (char *) realloc(*softLink, size);
		count = readlink(filePath, *softLink, size);
	}
	while(count > size);

	return count > 0;
}

/*
 * Get current processes
 * Input:
 * 			None
 * Output:
 * 			Print PID and command for each running process
 */
void getProcesses(void)
{
	int count, i;
	struct dirent **entries;
	char path[1024];
	char *softLink = NULL;

	count = scandir(PATH, &entries, filterDirectories, NULL);

	// Check if any errors occurred
	if(count < 0)
	{
		perror("scandir");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < count; i++)
	{
		snprintf(path, sizeof path, "%s%s%s%s", PATH, "/", entries[i]->d_name, "/exe");
		if(getSoftLink(path, &softLink))
			printf("%s %s\n", entries[i]->d_name, softLink);
	}
}

void run(void)
{
	getProcesses();
}
