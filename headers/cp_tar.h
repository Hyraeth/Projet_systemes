#include "tsh_fun.h"
#include "tar_fun.h"
#include "tar.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

int cpTar(pathStruct *pathData, pathStruct *pathLocation, int op, char *name);
int cpyDataFileNotInTar(char *path, char *data, struct posix_header *ph);
int copyFolder(struct pathStruct *pathData, struct pathStruct *pathLocation, char *name, struct posix_header *ph);
char *concatPathBeforeTarPathTar(char **pathBefore, char *name, int op);
char *getLast(char **charArray);
char *fileDataNotInTar(char *path, struct posix_header *ph);
void remplirHeader(struct posix_header *ph, struct stat sb);
void makePermissions(struct posix_header *ph, struct stat sb);
struct pathStruct *makeNewLocationStruct(pathStruct *pathLocation, char *name, int folder_exist);
void freeStruct(struct pathStruct *path);