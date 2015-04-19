#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <libgen.h>
#include <limits.h>
#include "index.h"

#define MAX_DIR_SIZE PATH_MAX
#define WORDS_FILE "words.txt"
#define TEMP_FOLDER "temp/"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: index DIR\n");
		return 1;
	}

	if (create_temp_folder()) return 1;

	char curr_dir[MAX_DIR_SIZE + 1];
	strcpy(curr_dir, dirname(realpath(argv[0],NULL)));

	char folder[MAX_DIR_SIZE + 1];
	getcwd(folder, MAX_DIR_SIZE + 1);
	strcat(folder, "/");
	strcat(folder, argv[1]);
	char *folder2 = realpath(folder, NULL);

	pid_t *pids = NULL;
	int num_pids;
	if (iterate_dir_files(curr_dir, folder2, &pids, &num_pids)) return 1;
	free(folder2);
	size_t i;
	for (i = 0; i < num_pids; ++i)
	{
		int status;
		if (waitpid(pids[i], &status, 0) == -1) return 1;
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;
	}

	free(pids);

	if (csc(curr_dir)) return 1;

	return delete_temp_folder(curr_dir);
}

int create_temp_folder()
{
	struct stat st = {0};
	if (stat(TEMP_FOLDER, &st) == -1) {
		if (mkdir(TEMP_FOLDER, 0700) == -1) return 1;
		else return 0;
	}
	return 0; // temp folder already exists
}

int delete_temp_folder(const char *curr_dir)
{
	pid_t pid = fork();
	switch (pid)
	{
	case -1: // error
		return 1;
	case 0: // child
	{
		if (execl("/bin/rm", "rm", "-rf", TEMP_FOLDER, NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
		int status;
		if (wait(&status) == -1) return 1;
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;
	}
	}
	return 0;
}

int iterate_dir_files(const char *curr_dir, const char *folder, pid_t **pids, int *num_pids)
{
	*num_pids = 0;
	DIR *dir = opendir(folder);
	struct dirent *ent;
	while((ent = readdir(dir)) != NULL)
	{
		if(ent->d_type == DT_REG) // regular files only
		{
			if (strcmp(ent->d_name, WORDS_FILE) == 0) continue; // Ignore words.txt
			if (ent->d_name[0] == '.') continue; // Ignore files starting with a dot
			if (ent->d_name[strlen(ent->d_name) - 1] == '~') continue; // Ignore files ending with a ~

			if ((*pids = realloc(*pids, ++*num_pids * sizeof(pid_t))) == NULL) return 1;

			char str[strlen(folder) + strlen(ent->d_name) + 1];
			strcpy(str, folder);
			strcat(str, "/");
			strcat(str, ent->d_name);

			if (sw(curr_dir, str, &(*pids)[*num_pids - 1])) return 1;
		}
	}
	if (closedir(dir) == -1) return 1;
	return 0;
}

int sw(const char *curr_dir, const char *file, pid_t *pid)
{
	*pid = fork();

	switch (*pid)
	{
	case -1: // error
		return 1;
	case 0: // child
	{
		char str[strlen(curr_dir) + strlen("sw") + 1];
		strcpy(str, curr_dir);
		strcat(str, "/");
		strcat(str, "sw");
		if (execl(str, "sw", file, NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
	}
	}
	return 0;
}

int csc(const char *curr_dir)
{
	pid_t pid = fork();
	switch (pid)
	{
	case -1: // error
		return 1;
	case 0: // child
	{
		char str[strlen(curr_dir) + strlen("csc") + 1];
		strcpy(str, curr_dir);
		strcat(str, "/");
		strcat(str, "csc");
		if (execl(str, "csc", NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
		int status;
		if (wait(&status) == -1) return 1;
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return 1;
	}
	}
	return 0;
}
