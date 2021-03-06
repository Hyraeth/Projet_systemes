#include "../headers/mv_tar.h"

/**
 * @brief makes the mv command with tar implicated in pathSrc or pathLocation
 * 
 * @param pathSrc : structure for the source path
 * @param pathLocation : strucutre for the location path
 * @return int : 1 if mv was successful else 0
 */
int mvWithTar(pathStruct *pathSrc, pathStruct *pathLocation)
{
    if (pathSrc->isTarBrowsed)
    {
        if (!doesTarExist(pathSrc->path))
        {
            printMessageTsh(STDERR_FILENO, "Un chemin impliquant un tar n'existe pas");
            return -1;
        }
    }
    if (pathLocation->isTarBrowsed)
    {
        if (!doesTarExist(pathLocation->path))
        {
            printMessageTsh(STDERR_FILENO, "Un chemin impliquant un tar n'existe pas");
            return -1;
        }
    }

    if (pathSrc->isTarBrowsed)
    {
        char c = typeFile(pathSrc->path, pathSrc->nameInTar);
        if (c == '9')
        {
            printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le fichier que vous voulez déplacer ou renommer existe bien");
            return -1;
        }
        else if (c == '5')
        {
            if (isEmptyDirTar(pathSrc->path, pathSrc->nameInTar))
            {
                if (pathLocation->isTarBrowsed)
                {
                    char c = typeFile(pathLocation->path, pathLocation->nameInTar);
                    if (c == '5')
                    {
                        printMessageTsh(STDERR_FILENO, "Le cas des tars imbriqués n'est pas géré");
                        return -1;
                    }
                    else
                    {
                        if (subFolderExistInTar(pathLocation->path, pathLocation->nameInTar))
                        {
                            if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                            {
                                return -1;
                            }
                            return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                        }
                        else
                        {
                            printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                            return -1;
                        }
                    }
                }
                else if (pathLocation->isTarIndicated)
                {
                    if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                    {
                        return -1;
                    }
                    return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                }
                else
                {
                    struct stat buffer;
                    if (stat(pathLocation->path, &buffer) != 0)
                    {
                        if (subFolderExistNotInTar(pathLocation->path))
                        {
                            if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                            {
                                return -1;
                            }
                            return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                        }
                    }
                    if (S_ISDIR(buffer.st_mode))
                    {
                        if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                        {
                            return -1;
                        }
                        return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                    }
                    else
                    {
                        printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                        return -1;
                    }
                }
            }
            else
            {
                printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le dossier que vous voulez déplacer est vide");
                return -1;
            }
        }
        else
        {
            if (pathLocation->isTarBrowsed)
            {
                char cLocation = typeFile(pathLocation->path, pathLocation->nameInTar);
                if (cLocation == '5')
                {
                }
                else if (cLocation == '9')
                {
                    if (isInSameFolder(pathSrc, pathLocation))
                    {
                        return renameInTar(pathSrc->path, pathSrc->nameInTar, pathLocation->nameInTar);
                    }
                    else if (subFolderExistInTar(pathLocation->path, pathLocation->nameInTar))
                    {
                        if (cpTar(pathSrc, pathLocation, 0, pathSrc->name) == -1)
                        {
                            return -1;
                        }
                        return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                    }
                }
                else
                {
                    printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }
            else if (!pathLocation->isTarIndicated)
            {
                struct stat buffer;
                if (stat(pathLocation->path, &buffer) != 0)
                {
                    if (subFolderExistNotInTar(pathLocation->path))
                    {
                        if (cpTar(pathSrc, pathLocation, 0, pathSrc->name) == -1)
                        {
                            return -1;
                        }
                        return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
                    }
                    else
                    {
                        printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                        return -1;
                    }
                }

                else if (!S_ISDIR(buffer.st_mode))
                {
                    printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }

            //Si on ne se retrouve pas dans les cas du dessus alors on veut bien déplacer le fichier dans un dossier
            if (cpTar(pathSrc, pathLocation, 0, pathSrc->name) == -1)
            {
                return -1;
            }
            return deleteFileInTar(pathSrc->nameInTar, pathSrc->path);
        }
    }
    else if (pathSrc->isTarIndicated)
    {
        if (!isEmptyTar(pathSrc->path))
        {
            printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le dossier que vous voulez déplacer est vide");
            return -1;
        }

        if (pathLocation->isTarBrowsed)
        {
            printMessageTsh(STDERR_FILENO, "Le cas des tars imbriqués n'est pas géré");
            return -1;
        }
        else if (pathLocation->isTarIndicated)
        {
            if (subFolderExistNotInTar(pathLocation->path))
            {
                if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                {
                    return -1;
                }
                if (remove(pathSrc->path) == 0)
                    return 1;
                else
                    return 0;
            }
        }
        else
        {
            struct stat buffer;
            if (stat(pathSrc->path, &buffer) != 0 || !S_ISDIR(buffer.st_mode))
            {
                printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                return -1;
            }
            else
            {
                if (cpTar(pathSrc, pathLocation, 0, pathSrc->name) == -1)
                {
                    return -1;
                }
                if (remove(pathSrc->path) == 0)
                    return 1;
                else
                    return 0;
            }
        }
    }
    else
    {
        struct stat buffer;
        if (stat(pathSrc->path, &buffer) != 0)
        {
            printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le fichier que vous voulez déplacer ou renommer existe bien");
            return -1;
        }
        if (S_ISDIR(buffer.st_mode))
        {
            DIR *dir;
            if ((dir = opendir(pathSrc->path)) == NULL)
            {
                perror("tsh: mv");
            }
            struct dirent *dirent;
            while ((dirent = readdir(dir)) != NULL)
            {
                if (strcmp(".", dirent->d_name) != 0 && strcmp("..", dirent->d_name) != 0)
                {
                    printMessageTsh(STDERR_FILENO, "Veuillez vérifier que le dossier que vous voulez déplacer est vide");
                    closedir(dir);
                    return -1;
                }
            }
            closedir(dir);
            if (pathLocation->isTarBrowsed)
            {
                char c = typeFile(pathLocation->path, pathLocation->nameInTar);
                if (c == '5')
                {
                    if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                    {
                        return -1;
                    }
                    if (remove(pathSrc->path) == 0)
                        return 1;
                    else
                        return 0;
                }
                else if (c == '9')
                {
                    if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                    {
                        return -1;
                    }
                    if (remove(pathSrc->path) == 0)
                        return 1;
                    else
                        return 0;
                }
                else
                {
                    printMessageTsh(STDERR_FILENO, "Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }
            else if (pathLocation->isTarIndicated)
            {
                if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                {
                    return -1;
                }
                if (remove(pathSrc->path) == 0)
                    return 1;
                else
                    return 0;
            }
            //Cas si pathLocation->isTarIndicated == 0 déjà géré
        }
        else
        {
            if (pathLocation->isTarBrowsed)
            {
                char c = typeFile(pathLocation->path, pathLocation->nameInTar);
                if (c == '5')
                {
                    if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                    {
                        return -1;
                    }
                    if (remove(pathSrc->path) == 0)
                        return 1;
                    else
                        return 0;
                }
                else
                {
                    if (subFolderExistInTar(pathLocation->path, pathLocation->nameInTar))
                    {
                        if (cpTar(pathSrc, pathLocation, 0, pathSrc->name) == -1)
                        {
                            return -1;
                        }
                        return remove(pathSrc->path);
                    }
                }
            }
            else if (pathLocation->isTarIndicated)
            {
                if (cpTar(pathSrc, pathLocation, 1, pathSrc->name) == -1)
                {
                    return -1;
                }
                if (remove(pathSrc->path) == 0)
                    return 1;
                else
                    return 0;
            }
        }
    }
    return -1;
}

/**
 * @brief checks if the path of the two structures are in the same folder
 * 
 * @param pathSrc : path to be compared 1
 * @param pathLocation : path to be compared 1
 * @return int 1 if they are in the same folder, else 0
 */
int isInSameFolder(pathStruct *pathSrc, pathStruct *pathLocation)
{
    if (pathSrc->isTarBrowsed && pathLocation->isTarBrowsed)
    {
        if (strcmp(pathSrc->path, pathLocation->path) == 0)
        {
            return comparePath(pathSrc->nameInTar, pathSrc->path);
        }
        else
            return 0;
    }
    else if (!(pathSrc->isTarBrowsed || pathLocation->isTarBrowsed))
    {
        return comparePath(pathLocation->path, pathSrc->path);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief checks if two path are in the same folder
 *
 * @return int : 1 if they are, 0 if not
 */
int comparePath(char *path1, char *path2)
{
    int nbSlash1 = nbSlash(path1);
    int nbSlash2 = nbSlash(path2);

    if (nbSlash1 != nbSlash2)
        return 0;

    int nbSlashCrossed = 0;

    int min = (strlen(path2) > strlen(path1)) ? strlen(path1) : strlen(path2);
    for (size_t i = 1; i < min; i++)
    {
        if (path1[i] != path2[i])
        {
            return 0;
        }
        if (path2[i] == '/') //if all slashes have been crossed in the loop and there has been no difference in the strings then they are in the same folder
        {
            nbSlashCrossed++;
            if (nbSlashCrossed == nbSlash1)
                return 1;
        }
    }
    return 0;
}

/**
 * @brief counts the number of slashs in a path
 */
int nbSlash(char *path)
{
    int res = 0;
    for (size_t i = 1; i < strlen(path) - 1; i++)
    {
        if (path[i] == '/')
            res++;
    }
    return res;
}