#include "tsh_fun.h"
#include "tar_fun.h"
#include "cp_tar.h"
#include "rm_tar.h"
#include "rmdir_tar.h"

int mvWithTar (pathStruct *pathSrc, pathStruct *pathLocation);
int isInSameFolder (pathStruct *pathSrc, pathStruct *pathLocation);
int comparePath (char *path1, char *path2);
int nbSlash (char *path);