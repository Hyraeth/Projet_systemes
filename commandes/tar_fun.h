#ifndef TAR_FUN_H
#define TAR_FUN_H

char **parse_path(char *path) ;
char **path_to_tar_file_path (char *path);
int isTar(char *file);
int copyFileInTar (int fd_src, const char *name, int fd_dest);
char *fileDataInTar (char *name_file, char *path_tar);
int isTarFolder (char *folder, char**path);
char typeFile (char *path_tar, char *pathInTar) ;
int deleteFileInTar (char *name_file, char *path_tar);

#endif //CMAKE_TESTAPP_TAR_FUN_H
