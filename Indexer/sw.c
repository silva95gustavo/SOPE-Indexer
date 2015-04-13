#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>

#define READ 0
#define WRITE 1

#define BUF_SIZE 2048
#define WORDS_FILE "words.txt"
#define TEMP_FOLDER "temp/"

int find_words(const char *file);
int find_word(const char *word, const char *file, FILE *fp);
int grep(const char *word, const char *file, char *buf);
int get_chapter_num(const char *file, unsigned *chapter_num);
int get_directory_name(const char *file, char *directory);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: sw FILE\n");
		return 1;
	}

	if (find_words(argv[1])) return 1;

	return 0;
}

int find_words(const char *file)
{
	char directory[strlen(file) + 1];
	get_directory_name(file, directory);
	char words[strlen(directory) + strlen(WORDS_FILE) + 1];
	strcpy(words, directory);
	strcat(words, WORDS_FILE);
	FILE *fpr = fopen(words, "r");
	if (fpr == NULL) return 1;

	char filecpy[strlen(file) + 1];
	strcpy(filecpy, file);

	char dest[strlen(TEMP_FOLDER) + strlen(filecpy) + 1];
	strcpy(dest, TEMP_FOLDER);
	strcat(dest, basename(filecpy));

	FILE *fpw = fopen(dest, "w");
	if (fpw == NULL) return 1;
	char buf[BUF_SIZE];
	while (fgets(buf, sizeof(buf), fpr))
	{
		buf[strcspn(buf, "\n")] = '\0'; // Remove trailing newline
		if (find_word(buf, file, fpw)) return 1;
	}
	if (ferror(fpr)) return 1;

	fclose(fpr);
	fclose(fpw);

	return 0;
}

int find_word(const char *word, const char *file, FILE *fp)
{
	char buf[BUF_SIZE];
	buf[0] = '\0';
	grep(word, file, buf);

	char *str;
	if ((str = strtok(buf, "\n")) == NULL) return 0; // return 0 because the output may be empty (if the word is not found)

	unsigned chapter_num;
	get_chapter_num(file, &chapter_num);

	fprintf(fp, "%s:", word);

	while (str != NULL)
	{
		unsigned line_number;
		if (sscanf(str, "%u:%*s", &line_number) < 0) return 1;
		fprintf(fp, " %d-%d", chapter_num, line_number);

		str = strtok(NULL, "\n");
	}
	fprintf(fp, "\n");
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
	if ((result = read(fd_pipe[READ], buf, BUF_SIZE - 2)) == -1) return 1;
	buf[result] = '\0';

	if (close(fd_pipe[READ]) == -1) return 1;

	return 0;
}

int get_chapter_num(const char *file, unsigned *chapter_num)
{
	char filecpy[strlen(file) + 1];
	strcpy(filecpy, file);
	char *base = basename(filecpy);
	if (sscanf(base, "%u.%*s", chapter_num) < 0) return 1;
	return 0;
}

int get_directory_name(const char *file, char *directory)
{
	char filecpy[strlen(file) + 1];
	strcpy(filecpy, file);
	char *dir = dirname(filecpy);
	strcpy(directory, dir);
	strcat(directory, "/");
	return 0;
}
