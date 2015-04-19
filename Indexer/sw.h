#ifndef SW_H
#define SW_H

int find_words(const char *file);
int find_word(const char *word, const char *file, FILE *fp);
int grep(const char *word, const char *file, char *buf);
int get_chapter_num(const char *file, unsigned *chapter_num);
int get_directory_name(const char *file, char *directory);

#endif
