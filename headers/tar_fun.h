#ifndef TAR_FUN_H
#define TAR_FUN_H

#include "tar.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>


char **parse_path(char *path) ;
int isTar(char *file);
int copyFileInTar (char *dataToCopy, char *name, char *path_to_tar);
char *fileDataInTar (char *name_file, char *path_tar);
int isTarFolder (char *folder, char**path);
char typeFile (char *path_tar, char *pathInTar) ;
int deleteFileInTar (char *name_file, char *path_tar);

#endif //TAR_FUN_H
