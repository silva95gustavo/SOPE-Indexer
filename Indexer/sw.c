#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define READ 0
#define WRITE 1

#define BUF_SIZE 20

int main(int argc, char *argv[])
{
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
		if (execl("/bin/grep", "grep", "-nob", argv[1], argv[2], NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
		break;
	}
	}

	if (close(fd_pipe[WRITE]) == -1) return 1;

	char buf[BUF_SIZE];

	int result;
	do
	{
		if ((result = read(fd_pipe[READ], buf, BUF_SIZE - 1)) == -1) return 1;
		if (write(STDOUT_FILENO, buf, result) < result) return 1;
	} while (result == BUF_SIZE - 1);

	if (close(fd_pipe[READ]) == -1) return 1;

	return 0;
}
