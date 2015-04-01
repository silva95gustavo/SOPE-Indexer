#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

#define BUF_SIZE 2048

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: sw WORD FILE\n");
		return 1;
	}

	int fd_pipe[2];
	if (pipe(fd_pipe) < 0) return 1;

	pid_t pid = fork();

	switch (pid)
	{
	case -1: // error
		return 1;
	case 0: // child
	{
		if (close(fd_pipe[READ]) == -1) return 1;
		if (dup2(fd_pipe[WRITE], STDOUT_FILENO) == -1) return 1;
		if (execl("/bin/grep", "grep", "-no", argv[1], argv[2], NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
		break;
	}
	}

	if (close(fd_pipe[WRITE]) == -1) return 1;

	int status;
	if (wait(&status) == -1) return 1;
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;

	char buf[BUF_SIZE];

	int result;
	if ((result = read(fd_pipe[READ], buf, BUF_SIZE - 1)) == -1) return 1;

	char *str;
	if ((str = strtok(buf, "\n")) == NULL) return 1;

	printf("%s: ", argv[1]);

	while (str != NULL)
	{
		if ((str = strtok(NULL, ":")) == NULL) return 1;
		unsigned line_number = atoi(str);
		if ((str = strtok(NULL, "\n")) == NULL) return 1;
		printf("%d ", line_number);
	}

	if (close(fd_pipe[READ]) == -1) return 1;

	return 0;
}
