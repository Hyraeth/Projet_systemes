#include "../headers/mv_tar.h"

int mvWithTar (pathStruct *pathSrc, pathStruct *pathLocation) {
    if (pathSrc->isTarBrowsed) {
        char c = typeFile(pathSrc->path,pathSrc->nameInTar);
        if (c == '9') {
            printMessageTsh(STDERR_FILENO,"Veuillez vérifier que le fichier que vous voulez déplacer ou renommer existe bien");
            return -1;
        }
        else if (c == '5') {
            if (isEmptyDirTar(pathSrc->path,pathSrc->nameInTar)) {
                if (pathLocation->isTarBrowsed) {
                    char c = typeFile(pathLocation->path,pathLocation->nameInTar);
                    if (c == '5') {
                        //TODO cp and rm
                    }
                    else {
                        printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                        return -1;
                    }
                }
                else if (pathLocation->isTarIndicated) {
                    //TODO tar in tar
                }
                else {
                    struct stat *buffer;
                    if (stat(pathLocation->path,buffer) != 0) {
                        printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                        return -1;
                    }
                    if (S_ISDIR(buffer->st_mode)) {
                        //TODO cp and rm
                    }
                    else {
                        printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                        return -1;
                    }
                }
            }
            else {
                printMessageTsh(STDERR_FILENO,"Veuillez vérifier que le dossier que vous voulez déplacer est vide");
                return -1;
            }
        }
        else {

        }
    }
    else if (pathSrc->isTarIndicated) {
        //TODO 
    }
    else {
        struct stat *buffer;
        if (stat(pathSrc->path,buffer) != 0) {
            printMessageTsh(STDERR_FILENO,"Veuillez vérifier que le fichier que vous voulez déplacer ou renommer existe bien");
            return -1;
        }
        if (S_ISDIR(buffer->st_mode)) {
            DIR *dir = opendir(pathLocation->path);
			struct dirent *dirent;

			while ((dirent = readdir(dir)) != NULL)
			{
				if (strcmp(".", dirent->d_name) != 0 && strcmp("..", dirent->d_name) != 0)
				{
					printMessageTsh(STDERR_FILENO,"Veuillez vérifier que le dossier que vous voulez déplacer est vide");
                    closedir(dir);
                    return -1;
				}
			}
			closedir(dir);

            if (pathLocation->isTarBrowsed) {
                char c = typeFile(pathLocation->path,pathLocation->nameInTar);
                if (c == '5') {
                    //TODO rm and cp
                }
                else {
                    printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }
            else if (pathLocation->isTarIndicated) {
                //TODO rm and cp
            }
            else {
                struct stat *buffer2;
                if (stat(pathLocation->path,buffer2) != 0) {
                    printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
                if (S_ISDIR(buffer2->st_mode)) {
                    //TODO cp and rm
                }
                else {
                    printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }
        }
        else {
            struct stat *buffer2;
            if (stat(pathLocation->path,buffer2) != 0) {
                if (isInSameFolder(pathLocation,pathSrc)) {
                    if (rename(pathSrc->path,pathLocation->path) == 0) {
                        return 1;
                    }
                    else return -1;
                }
                else {
                    printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                    return -1;
                }
            }
            if (S_ISDIR(buffer2->st_mode)) {
                //TODO rm and cp
            }
            else {
                printMessageTsh(STDERR_FILENO,"Erreur avec les chemins spécifiés, veuillez vérifier");
                return -1;
            }
        }
    }
}

int isInSameFolder (pathStruct *pathSrc, pathStruct *pathLocation) {
    if (pathSrc->isTarBrowsed && pathLocation->isTarBrowsed) {
        if (strcmp(pathSrc->path,pathLocation->path) == 0){
            return comparePath(pathSrc->nameInTar,pathSrc->path);
        }
        else return 0;
    }
    else if (!(pathSrc->isTarBrowsed || pathLocation->isTarBrowsed)) {
        return comparePath(pathLocation->path,pathSrc->path);
    }
    else {
        return 0;
    }
}

int comparePath (char *path1, char *path2) {
    int nbSlash1 = nbSlash(path1);
    int nbSlash2 = nbSlash(path2);

    if (nbSlash1 != nbSlash2) return 0;

    int nbSlashCrossed = 0;

    int min = (strlen(path2) > strlen(path1)) ? strlen(path1) : strlen(path2);
    for (size_t i = 1; i < min; i++)
    {
        if (path1[i] != path2[i]) {
            return 0;
        }
        if (path2[i] == '/') {
            nbSlashCrossed ++;
            if (nbSlashCrossed == nbSlash1) return 1;
        }
    }
    return 0;
}

int nbSlash (char *path) {
    int res = 0;
    for (size_t i = 1; i < strlen(path) - 1; i++)
    {
        if (path[i] == '/') res ++;
    }
    return res;
}