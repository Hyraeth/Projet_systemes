#include "tsh_fun.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char ***parsePathAbsolute (char *path) {
    char *pathHere = get_current_dir_name();
    int size_1;
    char **pathHereArray = parse_path_array(pathHere,&size_1);
    free(pathHere);
    int size_2;
    char **pathArray = parse_path_array(path,&size_2);

    int i = 0;
    int nbElementsBefore_1 = size_1 - 1;
    int nbElementsBefore_2 = 0;
    while (pathArray[i] != NULL) {
        if (strcmp(pathArray[i],"..") == 0) {
            if (nbElementsBefore_2 == 0) {
                if (nbElementsBefore_2 == 0) {
                    perror("tsh");
                }
                else {
                    realloc(pathHereArray,nbElementsBefore_1);
                    pathHereArray[nbElementsBefore_1 - 1] = NULL;
                    nbElementsBefore_1 -- ;
                }
            }
            else {
                memmove(&pathArray[nbElementsBefore_2 - 1],
                        &pathArray[nbElementsBefore_2],
                        (size_1 - 1 - nbElementsBefore_2)*sizeof( char *) );
                realloc(pathHereArray,size_1 - 1);
                size_1 --;
                nbElementsBefore_2 --;
            }
        }
    }
    char ***path_res = malloc( 2 * sizeof(char **));
    path_res[0] = pathHereArray;
    path_res[1] = pathArray;
    return path_res
}

char **parse_path_array(char *path, int *a) {
    int nbargs = 8;

    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(path, "/");

    int nbword = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh realloc parse path");
        }

        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh malloc parse path");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, "/");
    }
    args[nbword] = NULL;
    &a = nbword + 1;
    return args;
}