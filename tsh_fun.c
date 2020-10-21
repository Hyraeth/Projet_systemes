#define _GNU_SOURCE

#include "tsh_fun.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char **parsePathAbsolute (char *path) {
    char *pathHere = get_current_dir_name();
    int size_1;
    char **pathHereArray = parse_path_array(pathHere,&size_1);
    free(pathHere);
    int size_2;
    char **pathArray = parse_path_array(path,&size_2);

    int i = 0;
    int nbElementsBefore_2 = 0;

    while (pathArray[i] != NULL) {
        if (strcmp(pathArray[i],"..") == 0) {
            if (nbElementsBefore_2 == 0) {
                if ((size_1 - 1) == 0) {
                    perror("tsh no such file or directory parsePathAbsolute");
                }
                else {
                    if ((pathHereArray = realloc(pathHereArray,(size_1 - 1) * sizeof(char *) )) == NULL){
                    	perror ("tsh realloc parsePathAbsolute");
                	}
                	size_1--;
                    pathHereArray[size_1 - 1] = NULL;

                    memmove(pathArray, &pathArray[1], (size_2 - 1)*sizeof( char *) );

	                if ((pathArray = realloc(pathArray,(size_2 - 1) * sizeof(char *))) == NULL) {
	                	perror ("tsh realloc parsePathAbsolute");
	                }
	                size_2 --;
                }
            }
            else {
                memmove(&pathArray[nbElementsBefore_2 - 1],
                        &pathArray[nbElementsBefore_2],
                        (size_2 - 1 - nbElementsBefore_2)*sizeof( char *) );
                if ((pathArray = realloc(pathArray,(size_2 - 1) * sizeof(char *))) == NULL) {
                	perror ("tsh realloc parsePathAbsolute");
                }
                size_2 --;
                nbElementsBefore_2 --;
            }
        }
        else {
        	i++;
        }
    }
    if ((pathHereArray = realloc(pathHereArray,(size_1 + size_2) * sizeof(char *) )) == NULL){
    	perror ("tsh realloc parsePathAbsolute");
	}
	memcpy(&pathHereArray[size_1 - 1],pathArray, size_2 * sizeof( char * ));
	free(pathArray);
    return pathHereArray;
}

char **parse_path_array(char *path, int *a) {
    int nbargs = 8;

    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(path, "/");

    int nbword = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += 8;
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
    *a = nbword + 1;
    return args;
}

void printArray (char **path1) {
	int i = 0;
	while (path1[i] != NULL) {
		printf("/%s",path1[i]);
		i++;
	}
	printf("\n");
}

int main(int argc, char const *argv[])
{
	char **res = parsePathAbsolute(argv[1]);
	printArray(res);
	return 0;
}