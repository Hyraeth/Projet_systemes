#include "../headers/rmdir_tar.h"

int rmdirInTar(pathStruct *pathToDelete)
{
    char c = typeFile(pathToDelete->path, pathToDelete->nameInTar);
    if (c == '9')
    {
        printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le dossier que vous voulez supprimer existe bien");
        return -1;
    }

    if (c == '5')
    {
        return rmEmptyDirTar(pathToDelete->path, pathToDelete->nameInTar);
    }
    else
    {
        printMessageTsh(STDERR_FILENO, "Pour supprimer autre chose qu'un dossier, veuillez utiliser la commande rm");
        return -1;
    }
}

int rmdirTar(char *path)
{
    if (!doesTarExist(path))
    {
        printMessageTsh(STDERR_FILENO, "Le tar que vous voulez supprimer n'existe pas");
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
        printMessageTsh(STDERR_FILENO, "Le tar que vous voulez supprimer n'est pas vide");
    }
}