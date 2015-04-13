#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DIR_SIZE 254

int iterate_dir_files(const char *folder);
int sw(const char *curr_dir, const char *file);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: index DIR\n");
		return 1;
	}

	if (iterate_dir_files(argv[1])) return 1;

	return 0;
}

int iterate_dir_files(const char *folder)
{
	DIR *dir = opendir(folder);
	char *curr_dir = get_current_dir_name();
	struct dirent *ent = readdir(dir);
	while(ent != NULL)
	{
		if(ent->d_type == DT_REG) // all files in TEMP_FOLDER must be the temporary indexes created by sw
		{
			char str[strlen(folder) + strlen(ent->d_name) + 1];
			strcpy(str, folder);
			strcpy(str, ent->d_name);
			sw(curr_dir, ent->d_name);
		}
		ent = readdir(dir);
	}
	free(curr_dir);
	return 0;
}

int sw(const char *curr_dir, const char *file)
{
	pid_t pid = fork();

	switch (pid)
	{
	case -1: // error
		return 1;
	case 0: // child
	{
		char str[strlen(curr_dir) + strlen("sw") + 1];
		strcpy(str, curr_dir);
		strcat(str, "sw");

		if (execl(str, "sw", file, NULL) == -1) return 1;
		break; // <-- most useless break ever, but causes a compilation warning if removed
	}
	default: // parent
	{
		break;
	}
	}
	return 0;
}
