#ifndef CAT_TAR_H
#define CAT_TAR_H

#include "tar_fun.h"


int cat_tar(struct posix_header * header, int fd);
int cat(char *path_tar, char *path);

#endif //CAT_TAR_H