#include "../headers/rmdir_tar.h"

int rmdirInTar(pathStruct *pathToDelete)
{
    char c = typeFile(pathToDelete->path, pathToDelete->nameInTar);
    if (c == '9')
    {
        errno = ENOENT;
        perror("tsh: mkdir");
        return -1;
    }

    if (c == '5')
    {
        if (isEmptyDirTar(pathToDelete->path, pathToDelete->nameInTar))
        {
            return deleteFileInTar(pathToDelete->nameInTar, pathToDelete->path);
        }
        else
        {
            errno = ENOTEMPTY;
            perror("tsh: rmdir");
            return -1;
        }
    }
    else
    {
        errno = EINVAL;
        perror("tsh: rmdir");
        return -1;
    }
}

int rmdirTar(char *path)
{
    if (!doesTarExist(path))
    {
        errno = ENOENT;
        perror("tsh: mkdir");
        return -1;
    }

    if (isEmptyTar(path))
    {
        if (remove(path) == 0)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        errno = ENOTEMPTY;
        perror("tsh: rmdir");
    }
}