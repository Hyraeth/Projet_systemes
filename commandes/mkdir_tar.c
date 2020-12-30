#include "../headers/mkdir_tar.h"

/**
 * @brief creates a folder in a tar or an empty tar
 * 
 * @param pathSrc path structure for the folder we will create
 * @return int 1 if it was created, else 0
 */
int mkdirTar(pathStruct *pathSrc)
{
    int size;
    int res;

    if (!doesTarExist(pathSrc->path))
    {
        printMessageTsh(STDERR_FILENO, "Vérifiez que le tar où vous voulez créer le dossier existe bien");
        return -1;
    }

    char *copyNameInTar = malloc(strlen(pathSrc->nameInTar) + 1);
    strcpy(copyNameInTar, pathSrc->nameInTar);
    char **arrInTarName = parse_path_array(copyNameInTar, &size);

    char *path = malloc(1);
    path[0] = '\0';

    if (size == 2)
    {
        if (typeFile(pathSrc->path, pathSrc->nameInTar) != '9') //if a file already has the name of the folder we want to create
        {
            errno = EEXIST;
            perror("tsh: mkdir");
            res = -1;
        }
        else
        {
            res = mkdirInTar(pathSrc->path, pathSrc->nameInTar, NULL);
        }
        freeArr2D(arrInTarName);
        free(copyNameInTar);
        return res;
    }

    for (size_t i = 0; i < size - 2; i++) //Create a path to the subfolder of the folder we want to create
    {
        if ((path = realloc(path, strlen(path) + strlen(arrInTarName[i]))) == NULL)
        {
            perror("tsh: mkdir");
            return -1;
        }
        strcat(path, arrInTarName[i]);
    }

    char c = typeFile(pathSrc->path, path);
    if (c == '5') //If the subfolder exists
    {
        if (typeFile(pathSrc->path, pathSrc->nameInTar) != '9') //if a file already has the name of the folder we want to create
        {
            errno = EEXIST;
            perror("tsh: mkdir");
            res = -1;
        }
        else
        {
            res = mkdirInTar(pathSrc->path, pathSrc->nameInTar, NULL);
        }
    }
    else if (c == '9') //If the subfolder doesn't exist
    {
        res = -1;
        errno = ENOENT;
        perror("tsh: mkdir");
    }
    else
    {
        res = -1;
        errno = ENOTDIR;
        perror("tsh: mkdir");
    }
    free(path);
    freeArr2D(arrInTarName);
    free(copyNameInTar);

    return res;
}

/**
 * @brief makes en empty tar
 * 
 * @param path : path to the tar we will create
 * @return int 1 if the tar we created else 0
 */
int mkTarEmpty(char *path)
{
    if (doesTarExist(path))
    {
        errno = EEXIST;
        perror("tsh: mkdir");
        return -1;
    }
    else
    {
        return makeEmptyTar(path);
    }
}
