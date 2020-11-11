#include "../headers/tsh_fun.h"
#include "../headers/tar_fun.h"
#include "../headers/tar.h"
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int cp_tar (char ***path1, char ***path2, int op);
int cpyDataFileNotInTar (char * path, char *data, struct posix_header *ph);
char *concatPathBeforeTarPathTar (char **pathBefore, char *name, int op, char *test);
char *getLast (char **charArray);
char *fileDataNotInTar (char *path,struct posix_header *ph);
void remplirHeader (struct posix_header *ph, struct stat *sb);