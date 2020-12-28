#include "../headers/mkdir_tar.h"

int mkdirTar(pathStruct *pathSrc)
{
    int size;
    int res;

    char *copyNameInTar = malloc(strlen(pathSrc->nameInTar) + 1);
    strcpy(copyNameInTar, pathSrc->nameInTar);
    char **arrInTarName = parse_path_array(copyNameInTar, &size);

    char *path = malloc(1);
    path[0] = '\0';

    if (size == 2)
    {
        res = mkdirInTar(pathSrc->path, pathSrc->nameInTar, NULL);
        freeArr2D(arrInTarName);
        free(copyNameInTar);
        return res;
    }

    for (size_t i = 0; i < size - 2; i++)
    {
        if ((path = realloc(path, strlen(path) + strlen(arrInTarName[i]))) == NULL)
        {
            perror("rm");
            return -1;
        }
        strcat(path, arrInTarName[i]);
    }

    char c = typeFile(pathSrc->path, path);
    if (c == '5')
    {
        res = mkdirInTar(pathSrc->path, pathSrc->nameInTar, NULL);
    }
    else if (c == '9')
    {
        res = -1;
        printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le dossier dans lequel vous voulez créer un autre dossier existe bien");
    }
    else
    {
        res = -1;
        printMessageTsh(STDERR_FILENO, "Vous essayez de créer un dossier dans un fichier qui n'est pas un dossier");
    }
    freeArr2D(arrInTarName);
    free(copyNameInTar);

    return res;
}
