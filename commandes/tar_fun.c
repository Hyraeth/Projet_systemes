#include "tar_fun.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int isTar(char *file) {
    if(strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
    return 0;
}


char **path_to_tar_file_path (char *path) {

   char *res = strstr(path,".tar");
   char **path_res = malloc(2*sizeof(char *));

   if (res == NULL) {
       path_res[0] = NULL;
       path_res[1] = path;
       return path_res
   }

   int len_res = strlen(res);
   int len_char_1 = strlen(path) - len_res + 4;
   char *path_to_tar = malloc(len_char_1 + 1);
   char *path_tar_to_file;

    strncpy (path_to_tar,path,len_char_1 + 1)

   if (path[len_char_1] == '/'){
       path_tar_to_file = malloc(len_res - 4);
       strncpy(path_tar_to_file,&path[len_char_1+1],len_res - 4);
   }
   else {
        path_tar_to_file = NULL
   }

    path_res[0] = NULL;
    path_res[1] = path;
    return path_res;

}


