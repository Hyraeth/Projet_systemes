#include "../headers/cp_tar.h"

/**
 * @brief Copy File or Folder with option -r or no option
 * 
 * @param pathData : strucuture for the path of the data to be copied
 * @param pathLocation : strucuture for the path of the folder where data is to be copied
 * @param op : 1 if there is the option -r, 0 if not
 * @param name : name of the file to be copied
 * @return int : return 1 if copy has been made, -1 if not
 */
int cpTar (pathStruct *pathData, pathStruct *pathLocation, int op, char *name) {
	char *dataToCopy;
	struct posix_header *ph = malloc(sizeof(struct posix_header));
	int res;

	if (pathData->isTarBrowsed) {
		dataToCopy = fileDataInTar(pathData->nameInTar,pathData->path,ph);
		if (typeFile(pathData->path,pathData->nameInTar) == '5') {
			if (!op) {
				printMessageTsh("Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
				free(dataToCopy);
				free(ph);
				return -1;
			}
			else {
				if (pathData->nameInTar[strlen(name) - 1] != '/') {
					pathData->nameInTar = realloc(pathData->nameInTar,strlen(pathData->nameInTar) + 2);
					strcat(pathData->nameInTar,"/");
					res = copyFolder(pathData,pathLocation,name,ph);
				}
				else res = copyFolder(pathData,pathLocation,name,ph);
				free(dataToCopy);
				free(ph);
				return res;
			}
		}
	}
	else {
		dataToCopy = fileDataNotInTar(pathData->path,ph);
		struct stat sb;
		if (stat(pathData->path,&sb) != 0) {
			printMessageTsh("Erreur lors de l'ouverture d'un fichier");
			free(dataToCopy);
			free(ph);
			return -1;
		}
		if (S_ISDIR(sb.st_mode)) {
			if (!op) {
				printMessageTsh("Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
				free(ph);
				free(dataToCopy);
				return -1;
			}
			else {
				res = copyFolder(pathData,pathLocation,name,ph);
				free(dataToCopy);
				free(ph);
				return res;
			}
		}
	}


	if (pathLocation->isTarIndicated) {
		if (pathLocation->isTarBrowsed) {
			char *nameFull = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 1);
			strcpy(nameFull,pathLocation->nameInTar);
			strcat(nameFull,name);
			res = copyFileInTar(dataToCopy,nameFull,pathLocation->path,ph);
			free(nameFull);
		}
		else {
			res = copyFileInTar(dataToCopy,name,pathLocation->path,ph);
		}
	}
	else {
		res = cpyDataFileNotInTar(concatPathName(pathLocation->path,name),dataToCopy,ph);
	}

	free(dataToCopy);
	free(ph);
	return res;
}

/**
 * @brief Copy a folder and all the files in it
 * 
 * @param pathData : strucuture for the path of the data to be copied
 * @param pathLocation : strucuture for the path of the folder where data is to be copied
 * @param name : name of the folder to be copied
 * @param ph : structure posix_header 
 * @return int : return 1 if copy has been made, -1 if not
 */
int copyFolder (pathStruct *pathData, pathStruct *pathLocation, char *name, struct posix_header *ph) {
	int res;
	mode_t modeDir;

	if (!pathLocation->isTarIndicated) {
		char *pathLocationDir = concatPathName(pathLocation->path,name);
		res = mkdir(pathLocationDir,modeDir);
		free(pathLocationDir);
	}
	else {
		char *nameDirInTar = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 2);
		strcat(nameDirInTar,pathLocation->nameInTar);
		strcat(nameDirInTar,name);
		strcat(nameDirInTar,"/");
		mkdirInTar(pathLocation->path,nameDirInTar,ph);
		free(nameDirInTar);
	}

	if (res != -1) {
		pathStruct *pathLocationNew = makeNewLocationStruct(pathLocation,name);

		if (pathData->isTarBrowsed) {
			char **nameSubFiles = findSubFiles(pathData->path,pathData->nameInTar,1);
			int i = 0;
			while (nameSubFiles[i] != NULL)
			{
				pathStruct *pathDataNew = malloc(sizeof(pathStruct));
				pathDataNew->isTarBrowsed = 1;
				pathDataNew->isTarIndicated = 1;
				pathDataNew->nameInTar = malloc(strlen(pathData->nameInTar) + strlen(nameSubFiles[i]) + 2);

				strcpy(pathDataNew->nameInTar,pathData->nameInTar);
				strcat(pathDataNew->nameInTar,nameSubFiles[i]);

				pathDataNew->path = malloc(strlen(pathData->path) + 1);
				strcpy(pathDataNew->path, pathData->path);

				cpTar(pathDataNew,pathLocationNew,1,nameSubFiles[i]);
				free(nameSubFiles[i]);
				free(pathDataNew->path);
				free(pathDataNew->nameInTar);
				free(pathDataNew);
				i++;
			}
			
		}
		else {
			DIR *dir = opendir(pathData->path);
			struct dirent *dirent ;

			while ((dirent = readdir(dir)) != NULL)
			{
				if (strcmp(".",dirent->d_name) != 0 && strcmp("..",dirent->d_name) !=0) {
					pathStruct *pathDataNew = malloc(sizeof(pathStruct));
					pathDataNew->isTarBrowsed = 0;
					pathDataNew->isTarIndicated = 0;
					pathDataNew->nameInTar = NULL;
					pathDataNew->path = concatPathName(pathData->path,dirent->d_name);

					if (cpTar(pathDataNew,pathLocationNew,1,dirent->d_name) == -1) {
						freeStruct(pathLocationNew);
						closedir(dir);
						return -1;
					}
					free(pathDataNew->path);
					free(pathDataNew);
				}
			}
			closedir(dir);
		}
		freeStruct(pathLocationNew);
	}
	else {
		printMessageTsh("Erreur lors de la créature du dossier pour cp");
		return -1;
	}

	return 1;
}

/**
 * @brief Get the data from a file and fills its correspondant posix_header structure
 * 
 * @param path : path to the file we want to copy
 * @param ph : posix_header corresponding to that file
 * @return the data of the file in a char array
 */
char *fileDataNotInTar (char *path,struct posix_header *ph) {
	struct stat sb;
	if (stat(path,&sb) != 0) {
		printMessageTsh("Erreur lors de l'ouverture du fichier");
		return NULL;
	}

	int fd;
	if ((fd = open(path,O_RDONLY)) == -1) return NULL;

	remplirHeader(ph,sb);

	char *data = malloc(sb.st_size);
	read(fd,data,sb.st_size);
	close(fd);
	return data;
}

/**
 * @brief Fills the posix_header corresponding to a file
 * 
 * @param ph : posix_header to be filled
 * @param sb : stat structure of the fiole being copied
 */
void remplirHeader (struct posix_header *ph, struct stat sb) {
	makePermissions(ph,sb);
	sprintf(ph->size,"%011lo",sb.st_size);

	switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  ph->typeflag = '4';      break;
		case S_IFCHR:  ph->typeflag = '3';      break;
		case S_IFDIR:  ph->typeflag = '5';      break;
		case S_IFIFO:  ph->typeflag = '6';      break;
		case S_IFLNK:  ph->typeflag = '1';      break;
		case S_IFREG:  ph->typeflag = '0';      break;
		default:       printf("inconnu ?\n");   break;
    }

	memcpy(ph->magic, TMAGIC, strlen(TMAGIC));
	memcpy(ph->version, TVERSION, strlen(TVERSION));

	sprintf(ph->mtime,"%011lo",sb.st_mtime);
	sprintf(ph->uid,"%07d",sb.st_uid);
	sprintf(ph->gid,"%07d",sb.st_gid);

	struct passwd *pwd;
	pwd = getpwuid(sb.st_uid);
	memcpy(ph->uname, pwd->pw_name, strlen(pwd->pw_name));


	struct group *grp;
	grp = getgrgid(sb.st_gid);
	memcpy(ph->gname, grp->gr_name, strlen(grp->gr_name));
}

/**
 * @brief Fill rights in the posix_header strucutre 
 * 
 * @param ph : posix_header to be filled
 * @param sb : stat structure of the file we want to get the rights from
 */
void makePermissions (struct posix_header *ph, struct stat sb) {
	int permissions = 0;

	if( sb.st_mode & S_IRUSR )
        permissions += 400;
    if( sb.st_mode & S_IWUSR )
        permissions += 200;
    if( sb.st_mode & S_IXUSR )
        permissions += 100;

    if( sb.st_mode & S_IRGRP )
        permissions += 40;
    if( sb.st_mode & S_IWGRP )
        permissions += 20;
    if( sb.st_mode & S_IXGRP )
        permissions += 10;

    if( sb.st_mode & S_IROTH )
        permissions += 4;
    if( sb.st_mode & S_IWOTH )
        permissions += 2;
    if( sb.st_mode & S_IXOTH )
        permissions += 1;

	sprintf(ph->mode,"%07d",permissions);
}

/**
 * @brief If the path where we want to copy a file is a directory, then this function is used to create the make the copy
 * 
 * @param path path to the file we will create, essentially the path to the directory/name of the copied file
 * @param data data to be copied
 * @param ph posix_header corresponding to that file
 * @return 1 if the copy was successful, -1 if not
 */
int cpyDataFileNotInTar (char * path, char *data, struct posix_header *ph) {
	int fd;
	if ((fd = open(path, O_WRONLY | O_CREAT, 0644)) == -1 ) return -1;

	int n;

	n = write(fd,data, octalToDecimal(atoi(ph->size)));

	if ( n== -1 ) return -1;
	return 1;
}

/**
 * @brief Concatenate the path to a directory and the name of a file to create a path to the file we want to create 
 * 
 * @param pathBefore : path to a directory
 * @param name : name of the file
 * @param op 
 * @return the path to the file that is to be created 
 */
char *concatPathBeforeTarPathTar (char **pathBefore, char *name, int op) {
	char *path = array_to_path(pathBefore,op);
	if (strlen(path) == 0) return name;

	return concatPathName(path,name);
}

/**
 * @brief Get the name of the file we want to copy
 * 
 * @param charArray : array of a path, split with the "/"
 * @return the last string in the array, corresponding to the name of the file
 */
char *getLast (char **charArray) {
	int i = 0;
	while (charArray[i] != NULL) {
		i++;
	}
	if (i == 0) return NULL;
	else {
		char *res = malloc(strlen(charArray[i-1]) + 1);
		strcpy(res,charArray[i - 1]);
		return res;
	}
}

/**
 * @brief Build a new pathStruct when we try to copy files from a folder already copied
 * 
 * @param pathLocation : pathStruct for the folder that has been copied 
 * @param name : name of the folde that has been copied
 * @return pathStruct* : returns a pointer to the pathStruct created, 
 						used to copy the subfiles of the folder
 */
pathStruct *makeNewLocationStruct(pathStruct *pathLocation, char *name) {
	pathStruct *res = malloc(sizeof(pathStruct));
	res->isTarIndicated = pathLocation->isTarIndicated;
	res->name = NULL;

	if (res->isTarIndicated) {

		res->path = malloc(strlen(pathLocation->path) + 1);
		strcpy(res->path,pathLocation->path);

		char *nameDirInTar;
		if (pathLocation->isTarBrowsed){
			nameDirInTar = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 2);
			strcpy(nameDirInTar,pathLocation->nameInTar);
			strcat(nameDirInTar,name);
		} 
		else {
			nameDirInTar = malloc(strlen(name) + 2);
			strcpy(nameDirInTar,name);
		}
		strcat(nameDirInTar,"/");

		res->nameInTar = nameDirInTar;
		
		res->isTarBrowsed = 1;
	}

	else {
		res->isTarBrowsed = 0;
		res->nameInTar = NULL;
		res->path = concatPathName(pathLocation->path,name);
	}

	return res;
}

/**
 * @brief Free a 
 * 
 * @param path 
 */
void freeStruct (pathStruct *pathToFree) {
	if (pathToFree->nameInTar != NULL) free(pathToFree->nameInTar);
	free(pathToFree->path);
	if (pathToFree->name != NULL) free(pathToFree->name);
	free(pathToFree);
}
