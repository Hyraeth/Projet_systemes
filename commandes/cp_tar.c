#include "../headers/cp_tar.h"

/**
 * @brief General function to make copy file from path1 to directory or tar from path2
 * 
 * @param path1 : path to the file we want to copy 
 * @param path2 : path to the directory or tar where we want to copy
 * @param op : 1 if there is the option -r, else 0
 * @param beforeName : If we copy a directory, then path to get from the directory to the file we copy
 * @return 1 if copy was successful, -1 if there was an error 
 */
int cp_tar (char ***path1, char ***path2, int op,char *beforeName) {
	char *dataToCpy;
	struct posix_header *ph = malloc(sizeof(struct posix_header));
	int res;
	char *name;
	struct stat sb;

	if (path1[1] == NULL) {
		char *path = array_to_path(path1[0],1);

		if (stat(path,&sb) != 0) {
			printMessageTsh("Erreur lors de l'ouverture du fichier");
			return -1;
		}
		if (S_ISDIR(sb.st_mode)) {

		}

		if ((dataToCpy = fileDataNotInTar(path,ph)) == NULL) {
			return -1;
		}
		name = getLast(path1[0]);
		free(path);
	}
	else {
		char *path = concatPathBeforeTarPathTar(path1[0],path1[1][0],1);

		if (stat(path,&sb) != 0) {
			printMessageTsh("Erreur lors de l'ouverture du fichier");
			return -1;
		}
		if (S_ISDIR(sb.st_mode)) {
			
		}

		char *pathInTar = array_to_path(path1[2],0);
		if ((dataToCpy = fileDataInTar (pathInTar, path, ph)) == NULL) {
			return -1;
		}
		name = getLast(path1[2]);
		free(path);
		free(pathInTar);
	}

	if (path2[1] == NULL) {
		char *path = concatPathBeforeTarPathTar(path2[0],name,1);
		if (name == NULL) return -1;
		res = cpyDataFileNotInTar(path,dataToCpy,ph);
		free(path);
	}
	else {
		char *path = concatPathBeforeTarPathTar(path2[0],path2[1][0],1);
		char *nameInTar = concatPathBeforeTarPathTar(path2[2],name,0);
		res = copyFileInTar (dataToCpy, name, path,ph);
		free(path);
		free(nameInTar);
	}

	free(dataToCpy);
	free(ph);
	return res;
}

int cpTarTest (pathStruct *pathData, pathStruct *pathLocation, int op, char *name) {
	char *dataToCopy;
	struct posix_header *ph = malloc(sizeof(struct posix_header));
	int res;

	if (pathData->isTarBrowsed) {
		dataToCopy = fileDataInTar(pathData->nameInTar,pathData->path,ph);
		if (typeFile(pathData->path,pathData->nameInTar) == '5') {
			if (!op) {
				printMessageTsh("Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
				return -1;
			}
			else {
				copyFolder(pathData,pathLocation,name,ph);
			}
		}
	}
	else {
		dataToCopy = fileDataNotInTar(pathData->path,ph);
		struct stat sb;
		if (stat(pathData->path,&sb) != 0) {
			printMessageTsh("Erreur lors de l'ouverture du fichier");
			return -1;
		}
		if (S_ISDIR(sb.st_mode)) {
			if (!op) {
				printMessageTsh("Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
				return -1;
			}
			else {
				copyFolder(pathData,pathLocation,name,ph);
			}
		}
	}

	if (pathLocation->isTarBrowsed) {
		res = copyFileInTar(dataToCopy,pathLocation->nameInTar,pathLocation->path,ph);
	}
	else {
		res = cpyDataFileNotInTar(concatPathName(pathLocation->path,name),dataToCopy,ph);
	}

	return res;
}

int copyFolder (pathStruct *pathData, pathStruct *pathLocation, char *name, struct posix_header *ph) {
	int res;
	mode_t modeDir;

	if (!pathLocation->isTarIndicated) {
		char *pathLocationDir = concatPathName(pathLocation->path,name);
		res = mkdir(pathLocationDir,modeDir);
		free(pathLocationDir);
	}
	else {
		char *nameDirInTar = concatPathName(pathLocation->nameInTar,name);
		nameDirInTar = realloc(nameDirInTar, strlen(nameDirInTar) + 2);
		strcat(nameDirInTar,"/");
		char *data = malloc(1);
		data[0] = '\0';
		copyFileInTar(data,nameDirInTar,pathLocation->path,ph);
		free(data);
		free(nameDirInTar);
	}

	if (res != -1) {
		pathStruct *pathLocationNew = makeNewLocationStruct(pathLocation,name);

		if (pathData->isTarBrowsed) {

		}
		else {
			DIR *dir = opendir(pathData->path);
			struct dirent *dirent = malloc (sizeof(struct dirent));

			while ((dirent = readdir(dir)) != NULL)
			{
				pathStruct *pathDataNew = malloc(sizeof(pathStruct));
				pathDataNew->isTarBrowsed = 0;
				pathDataNew->isTarIndicated = 0;
				pathDataNew->nameInTar = NULL;
				pathDataNew->path = dirent->d_name;

				cpTarTest(pathDataNew,pathLocationNew,1,dirent->d_name);
				free(pathDataNew->path);
				free(pathDataNew);
			}
			free(dirent);
		}

		freeStruct(pathLocationNew);
		
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
	if (S_ISDIR(sb.st_mode)) {

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

	printf("Time %ld",sb.st_mtime);

	sprintf(ph->mtime,"%11ld",sb.st_mtime);
	sprintf(ph->uid,"%07d",sb.st_uid);
	sprintf(ph->gid,"%07d",sb.st_gid);

	struct passwd *pwd;
	pwd = getpwuid(sb.st_uid);
	memcpy(ph->uname, pwd->pw_name, strlen(pwd->pw_name));


	struct group *grp;
	grp = getgrgid(sb.st_gid);
	memcpy(ph->gname, grp->gr_name, strlen(grp->gr_name));
}

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
	n = write(fd,data, atoi(ph->size));

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
	else return charArray[i - 1];
}

pathStruct *makeNewLocationStruct(pathStruct *pathLocation, char *name) {
	pathStruct *res = malloc(sizeof(pathStruct));
	res->isTarBrowsed = pathLocation->isTarBrowsed;
	res->isTarIndicated = pathLocation->isTarIndicated;

	if (res->isTarIndicated) {

		res->path = malloc(strlen(pathLocation->path) + 1);
		memcpy(res->path,pathLocation->path,strlen(pathLocation->path));

		char *nameDirInTar = concatPathName(pathLocation->nameInTar,name);
		nameDirInTar = realloc(nameDirInTar, strlen(nameDirInTar) + 2);
		strcat(nameDirInTar,"/");
		res->nameInTar = nameDirInTar;
	}

	else {
		res->nameInTar = NULL;
		res->path = concatPathName(pathLocation->path,name);
	}
}

void freeStruct (pathStruct *path) {
	free(path->nameInTar);
	free(path->path);
	free(path);
}
