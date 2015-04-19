#ifndef INDEX_H
#define INDEX_H

int create_temp_folder();
int delete_temp_folder(const char *curr_dir);
int check_words_exist(const char *folder);
int iterate_dir_files(const char *curr_dir, const char *folder, pid_t **pids, int *num_pids);
int sw(const char *curr_dir, const char *file, pid_t *pid);
int csc(const char *curr_dir);

#endif
