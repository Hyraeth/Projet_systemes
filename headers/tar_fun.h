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
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "../headers/tsh_fun.h"


char **parse_path(char *path) ;
int isTar(char *file);
int copyFileInTar (char *dataToCopy, char *name, char *path_to_tar, struct posix_header *ph);
char *fileDataInTar (char *name_file, char *path_tar, struct posix_header *ph);
int isTarFolder (char *folder, char**path);
char typeFile (char *path_tar, char *pathInTar) ;
int deleteFileInTar (char *name_file, char *path_tar);
int rmWithOptionTar (char *path_to_tar, char *path_in_tar);
char *isSubFile (char *s, char *toVerify,int depth);
char **findSubFiles (char *path_tar, char *path_in_tar,int depth);
int strcmpTar (char *path_file, char *path_in_tar);
int octalToDecimal (long int octal);
long int decimalToOctal(long int decimalnum);
int mkdirInTar (char *path_tar, char *path_in_tar, struct posix_header *ph);
int makeEmptyTar (char *path);
int isEmptyTar (char *path);
int doesTarExist (char *path);
int rmEmptyDirTar (char *path_to_tar, char *path_in_tar);

#endif //TAR_FUN_H
