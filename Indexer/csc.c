#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define MAX_DIR_SIZE 254
#define MAX_BUF_SIZE 512
#define MAX_WORD_SIZE 50
#define END_WORD_CHAR ':'
#define TEMP_FOLDER "temp"					// temporary folder where files generated by sw are placed
#define UNSORTED_INDEX_NAME "us_index.txt"	// unsorted index, to be removed
#define SORTED_REPEAT_INDEX "sr_index.txt"	// sorted index, with repeated lines
#define INDEX_NAME "index.txt"				// sorted index, result

// Variables
DIR* dir;

// Function declarations
int setup_dir();
int concatenate_files_to_index(char** filenames, int filenumber);
void prep_filenames(char** filenames, int filenumber);
int sort_index();
int remove_repeated_lines();
int remove_temp_files();

int main(int argc, char *argv[])
{
	if (argc != 1)
	{
		printf("\nThis program takes no arguments\n\n");
		return 1;
	}

	if(setup_dir() != 0) return 1; // Set dir string

	struct dirent *ent;
	char** filenames = malloc(0);
	int filenumber = 0;

	ent = readdir(dir);
	while(ent != NULL)
	{
		if(ent->d_type == DT_REG) // all files in TEMP_FOLDER must be the temporary indexes created by sw
		{
			char** temp = (char**)malloc(++filenumber*sizeof(char*));
			memcpy(temp, filenames, (filenumber-1)*sizeof(char*));
			filenames[filenumber-1] = ent->d_name;
		}
		ent = readdir(dir);
	}
	// at this point, filenames contains the list of filenames to be read from DIR

	prep_filenames(filenames, filenumber);
	if(concatenate_files_to_index(filenames, filenumber) != 0) return 1;
	if(sort_index()!=0) return 1;
	if(remove_repeated_lines()!= 0) return 1;
	if(remove_temp_files()!=0) return 1;

	printf("\n\n\tDone\n\n");

	return 0;
}

int setup_dir()
{
	char *dir_name;

	dir_name = (char*)malloc(MAX_DIR_SIZE);
	dir_name = getcwd(dir_name, MAX_DIR_SIZE);

	if(dir_name == NULL) return 1;	// Error getting current dir

	int dir_size = strlen(dir_name);
	int temp_folder_size = strlen(TEMP_FOLDER);

	if(dir_size+2+temp_folder_size > MAX_DIR_SIZE) return 1;	// Complete path is too big

	dir_name[dir_size] = '/';
	int i;
	for(i = 0; i < temp_folder_size; i++)
	{
		dir_name[dir_size+1+i] = TEMP_FOLDER[i];
	}

	dir_size = strlen(dir_name);
	dir_name[dir_size++] = '/';

	dir = opendir(dir_name);

	if(dir == NULL) return 1;

	return 0;
}

int concatenate_files_to_index(char** filenames, int filenumber)
{
	char** cmd = (char**)malloc((filenumber+2)*sizeof(char*));
	cmd[0] = "cat";
	cmd[filenumber+1] = NULL;

	int i;
	for(i = 0; i < filenumber; i++)
		cmd[i+1] = filenames[i];

	int index_fd = open(UNSORTED_INDEX_NAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);

	if(index_fd < 0) return 1;

	int pid = fork();
	int status = 0;

	if(pid != 0)
	{
		dup2(index_fd, STDOUT_FILENO);
		execvp(cmd[0], cmd);
		exit(1);
	}
	else
		wait(&status);

	close(index_fd);
	if(status != 0) return 1;

	return 0;
}

void prep_filenames(char** filenames, int filenumber)
{
	int temp = strlen(TEMP_FOLDER);
	char *dir_str = malloc((1+temp)*sizeof(char));
	strcpy(dir_str, TEMP_FOLDER);
	dir_str[temp] = '/';		// dir_str becomes TEMP_FOLDED/, to be added to filenames for use in cat
	temp = strlen(dir_str);

	int i;
	for(i = 0; i < filenumber; i++)
	{
		int filename_length = strlen(filenames[i]);
		char *new_str = malloc((filename_length+temp)*sizeof(char));
		strcpy(new_str, dir_str);
		strcpy((new_str+filename_length), filenames[i]);
		filenames[i] = new_str;
	}
}

int sort_index()
{
	char **cmd = malloc(2);
	cmd[0] = "sort";
	cmd[1] = UNSORTED_INDEX_NAME;

	int index_fd = open(SORTED_REPEAT_INDEX, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);

	if(index_fd < 0) return 1;

	int pid = fork();
	int status = 0;

	if(pid == 0)
	{
		dup2(index_fd, STDOUT_FILENO);
		execvp(cmd[0], cmd);
		exit(1);
	}
	else
		wait(&status);

	close(index_fd);
	if(status != 0) return 1;

	return 0;
}

int remove_repeated_lines()
{
	FILE *repeat_file = fopen(SORTED_REPEAT_INDEX, "r");
	int index_fd = open(INDEX_NAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);

	if(!repeat_file) return 1;
	if(index_fd < 0) return 1;

	char str[MAX_BUF_SIZE];
	char word[MAX_WORD_SIZE];
	char *prev_word = NULL;

	int last_read_word = 0;		// 0 - false, 1 - true

	while(fscanf(repeat_file, "%s", str) != EOF)
	{
		int len = strlen(str);
		if(str[len-1] == END_WORD_CHAR)	// string read is a word of the index
		{
			last_read_word = 1;
			strcpy(word, str);

			if(prev_word == NULL)
			{
				write(index_fd, word, strlen(word)*sizeof(char));
				write(index_fd, " ", 1);

				prev_word = malloc(strlen(word));
				strcpy(prev_word, word);
			}
			else
			{
				if(strcmp(prev_word, word)!= 0)
				{
					prev_word = malloc(strlen(word));
					strcpy(prev_word, word);
					write(index_fd, "\n", 1);
					write(index_fd, word, strlen(word)*sizeof(char));
					write(index_fd, " ", 1);
				}
				else
					last_read_word = 0;
			}
		}
		else							// string read is a match <file>-<line>
		{
			if(last_read_word == 0)
				write(index_fd, ", ", 2);

			write(index_fd, str, strlen(str)*sizeof(char));

			last_read_word = 0;
		}

	}

	close(index_fd);
	fclose(repeat_file);

	return 0;
}

int remove_temp_files()
{
	if(remove(UNSORTED_INDEX_NAME) != 0) return 1;
	if(remove(SORTED_REPEAT_INDEX) != 0) return 1;

	return 0;
}

