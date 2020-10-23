#define _GNU_SOURCE

#include "headers/tsh_fun.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char **parsePathAbsolute (char *path, char *pwd) {
    int size_1;
    char **pwdArray = parse_path_array(pwd,&size_1);
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
                    if ((pwdArray = realloc(pwdArray,(size_1 - 1) * sizeof(char *) )) == NULL){
                    	perror ("tsh realloc parsePathAbsolute");
                	}
                	size_1--;
                    pwdArray[size_1 - 1] = NULL;

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
            nbElementsBefore_2++;
        	i++;
        }
    }
    if ((pwdArray = realloc(pwdArray,(size_1 + size_2) * sizeof(char *) )) == NULL){
    	perror ("tsh realloc parsePathAbsolute");
	}
	memcpy(&pwdArray[size_1 - 1],pathArray, size_2 * sizeof( char * ));
	free(pathArray);
    return pwdArray;
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

char ***path_to_tar_file_path_new (char **path) {

    int i = 0;
    while (path[i] != NULL) {
        i++;
    }
    int size = i;

    i = 0;
    int index_tar = -1;
    while (path[i] != NULL) {
        int lenpath_i = strlen(path[i]);
        if (lenpath_i > 4 && strcmp(&path[i][lenpath_i - 4],".tar") == 0) {
            index_tar = i ;
            break;
        }
        i++;
    }

    char ***path_res = malloc(3*sizeof(char **));

    if (index_tar == -1) {
        if (size == 0) {
            path_res[0] = NULL;
        }
        else {
            path_res[0] = (char **) malloc((size + 1) * sizeof(char *) );
            memcpy(path_res[0], path, size * sizeof(char *));
        }
        path_res[1] = NULL;
        path_res[2] = NULL;
        return path_res;
    }
    else {
        path_res[0] = (char **) malloc((index_tar + 1) * sizeof(char *) );
        memcpy(path_res[0], path, index_tar * sizeof(char *));
        path_res[0][index_tar] = NULL;
        path_res[1] = (char **) malloc(1 * sizeof(char *) );
        path_res[1][0] = path[index_tar];
        path_res[2] = (char **) malloc((size - index_tar) * sizeof(char *) );
        memcpy(path_res[2], &path[index_tar + 1], (size - index_tar) * sizeof(char *));
        path_res[2][size - index_tar - 1] = NULL;
    }
    return path_res;

}

void printArray (char **path1) {
	int i = 0;
	while (path1[i] != NULL) {
		printf("%s ",path1[i]);
		i++;
	}
	printf("\n");
}

char *array_to_path(char **array, int op) {
    if(array[0] == NULL) {
        char *path = malloc(1);
        path[0] = 0;
        return path;
    }
    int pathlength;
    if(op == 1) 
        pathlength = strlen(array[0])+2;
    else 
        pathlength = strlen(array[0])+1;
    char *path = malloc(pathlength);
    if(op == 1) {
        path[0] = '/';
        memcpy(path+1, array[0], strlen(array[0])+1);
    } else {
        memcpy(path, array[0], strlen(array[0])+1);
    }
        
    
    int i = 1;
    while (array[i] != NULL)
    {
        pathlength += strlen(array[i])+1;
        path = realloc(path, pathlength);
        strcat(path, "/");
        strcat(path, array[i]);
        i++;
    }
    path[strlen(path)] = '\0';
    return path;
}

int is_an_option(char *string) {
    return (string[0] == '-') ? 1 : 0;
}

void remove_escape_char(char *str) {
    for (size_t i = 0; str[i] != '\0'; i++)
    {
        if(str[i] == '\\') {
            memmove(str+i, str+i+1, strlen(str+i+1)+1);
            i--;
        }
    }
    
}

void remove_escape_char_array(char **array) {
    int i = 0;
    while(array[i] != NULL) {
        remove_escape_char(array[i]);
        i++;
    }
}
/*
int main(int argc, char const *argv[])
{
	char ***res = path_to_tar_file_path_new(parsePathAbsolute(argv[1],""));
	printArray(res[0]);
    printArray(res[1]);
    printArray(res[2]);
	return 0;
}
*/
