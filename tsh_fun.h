#ifndef TSH_FUN_H
#define TSH_FUN_H

char **parsePathAbsolute (char *path, char *pwd);
char **parse_path_array(char *path,int *a);
void printArray (char **path1);
char ***path_to_tar_file_path_new (char **path);

#endif //TSH_FUN_H
