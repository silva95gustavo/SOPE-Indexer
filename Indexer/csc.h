#ifndef CSC_H
#define CSC_H

int setup_dir(DIR **dir);
int concatenate_files_to_index(char** filenames, int filenumber, int pipefd[]);
void prep_filenames(char** filenames, int filenumber);
int* sort_index(int pipefd[], int newpipe[]);
int remove_repeated_lines(int pipefd[]);

#endif
