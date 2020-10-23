#ifndef TSH_FUN_H
#define TSH_FUN_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char **parsePathAbsolute (char *path, char *pwd);
char **parse_path_array(char *path,int *a);
void printArray (char **path1);
char ***path_to_tar_file_path_new (char **path);
char *array_to_path(char **array, int op);
int is_an_option(char *string);
void remove_escape_char(char *str);
void remove_escape_char_array(char **array);

#endif //TSH_FUN_H
