#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

#define BUF_SIZE 2048

int find_word(const char *word, const char *file);
int grep(const char *word, const char *file, char *buf);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: sw FILE\n");
		return 1;
	}

	if (find_word("Holmes", argv[1])) return 1;

	return 0;
}

int find_word(const char *word, const char *file)
{
	char buf[BUF_SIZE];
	grep(word, file, buf);

	char *str;
	if ((str = strtok(buf, "\n")) == NULL) return 1;

	printf("%s: ", word);

	while (str != NULL)
	{
		if ((str = strtok(NULL, ":")) == NULL) return 1;
		unsigned line_number = atoi(str);
		if ((str = strtok(NULL, "\n")) == NULL) return 1;
		printf("%d ", line_number);
	}

	return 0;
}

int grep(const char *word, const char *file, char *buf)
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
		if (execl("/bin/grep", "grep", "-no", word, file, NULL) == -1) return 1;
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

	int result;
	if ((result = read(fd_pipe[READ], buf, BUF_SIZE - 1)) == -1) return 1;

	if (close(fd_pipe[READ]) == -1) return 1;

	return 0;
}
