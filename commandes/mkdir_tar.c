#include "../headers/mkdir_tar.h"

int mkdir_tar (pathStruct *pathSrc) {
    int size;

    char **arrInTarName = parse_path_array(pathSrc->nameInTar,&size);

    char *path = malloc(1);
    path[0] = '\0';

    for (size_t i = 0; i < size - 2; i++)
    {
        if ((path = realloc(path,strlen(path) + strlen(arrInTarName[i]))) == NULL) {
            perror("rm");
            return -1;
        }
        strcat(path,arrInTarName[i]);
    }
    
    char c = typeFile(pathSrc->path,path);
    if (c == '5') {
        if ((path = realloc(path,strlen(path) + strlen(arrInTarName[size - 2]))) == NULL) {
            perror("rm");
            return -1;
        }
        strcat(path,arrInTarName[size - 2]);
    }
    else if (c == '9') {
        printMessageTsh("Veuillez vérifier que le dossier dans lequel vous voulez créer un autre dossier existe bien");
    }
    else {
        printMessageTsh("Vous essayez de créer un dossier dans un fichier qui n'est pas un dossier");
    }
}
