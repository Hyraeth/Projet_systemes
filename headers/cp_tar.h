#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int cp_tar (char ***path1, char ***path2, int op);
char *fileDataNotInTar (char *path);
int cpyDataFileNotInTar (char * path, char *data);
char *concatPathBeforeTarPathTar (char **pathBefore, char *name, int op, char *test);
char *getLast (char **charArray);