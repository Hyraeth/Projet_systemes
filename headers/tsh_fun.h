#ifndef TSH_FUN_H
#define TSH_FUN_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char **parsePathAbsolute (char *path, char *pathHere);
char **parse_path_array(char *path,int *a);
void printArray (char **path1);
char ***path_to_tar_file_path_new (char **path);
char *array_to_path(char **array);
int is_an_option(char *string);


#endif //TSH_FUN_H
