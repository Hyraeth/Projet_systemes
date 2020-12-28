#ifndef RM_TAR_H
#define RM_TAR_H

#include "rm_tar.h"
#include "tsh_fun.h"
#include "tar_fun.h"

int rm_in_tar (pathStruct *pathToDelete, int op);
int rm_tar (char *path);

#endif