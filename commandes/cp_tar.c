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
int cpTar(pathStruct *pathData, pathStruct *pathLocation, int op, char *name)
{
	char *dataToCopy;
	struct posix_header *ph = malloc(sizeof(struct posix_header));
	int res;

	if (pathData->isTarBrowsed) //if we want to copy somthing in a tar
	{
		if (!doesTarExist(pathData->path))
		{
			perror("tsh: cp");
			free(ph);
			return -1;
		}
		char typesrc = typeFile(pathData->path, pathData->nameInTar);
		if (typesrc == '9')
		{
			errno = ENOENT;
			perror("tsh: cp");
			free(ph);
			return -1;
		}
		dataToCopy = fileDataInTar(pathData->nameInTar, pathData->path, ph); //copy into a buffer the content of what we want to copy

		if (typesrc == '5') //if what we want to copy is a folder
		{
			if (!op) //if the option -r is not set
			{
				printMessageTsh(STDERR_FILENO, "tsh: cp: -r not specified");
				if (dataToCopy != NULL)
					free(dataToCopy);
				free(ph);
				return -1;
			}
			else //if the option -r has been set
			{
				if (pathData->nameInTar[strlen(pathData->nameInTar) - 1] != '/') // if the name of the directory to copy does not end with a '/'
				{
					pathData->nameInTar = realloc(pathData->nameInTar, strlen(pathData->nameInTar) + 2); //make more space
					strcat(pathData->nameInTar, "/");													 //add '/' at the end
					res = copyFolder(pathData, pathLocation, name, ph);									 //copy the directory
				}
				else													// if the name of the directory to copy does end with a '/
					res = copyFolder(pathData, pathLocation, name, ph); //copy the directory
				if (dataToCopy != NULL)									//after everything has been copied, free if not null
					free(dataToCopy);
				free(ph);
				return res;
			}
		}
	}
	else if (pathData->isTarIndicated)
	{
		if (!doesTarExist(pathData->path))
		{
			perror("tsh: cp");
			free(ph);
			return -1;
		}
		if (!op) //if -r has not been set
		{
			printMessageTsh(STDERR_FILENO, "Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
			free(ph);
			return -1;
		}
		dataToCopy = fileDataNotInTar(pathData->path, ph);
		if (pathLocation->isTarIndicated)
		{
			char *nameWithoutTar = malloc(strlen(pathData->name) - 3);
			strncpy(nameWithoutTar, pathData->name, strlen(pathData->name) - 4);
			nameWithoutTar[strlen(pathData->name) - 4] = '\0';
			ph->typeflag = '5';
			sprintf(ph->size, "%07d", 0);
			res = copyFolder(pathData, pathLocation, nameWithoutTar, ph); //copy folder
			if (dataToCopy != NULL)
				free(dataToCopy);
			free(ph);
			free(nameWithoutTar);
			return res;
		}
		else
		{
			res = copyFolder(pathData, pathLocation, name, ph); //copy folder
			if (dataToCopy != NULL)
				free(dataToCopy);
			free(ph);
			return res;
		}
	}
	else //if what we want to copy is not in a tar
	{
		dataToCopy = fileDataNotInTar(pathData->path, ph); //copy into a buffer what we want to copy
		struct stat sb;
		if (stat(pathData->path, &sb) != 0) //store info in the file/directory if possible,
		{
			perror("tsh: cp: cpTar: stat"); //file/directory probably dosen't exist
			if (dataToCopy != NULL)
				free(dataToCopy);
			free(ph);
			return -1;
		}
		if (S_ISDIR(sb.st_mode)) //if what we want to copy is a folder
		{
			if (!op) //if -r has not been set
			{
				printMessageTsh(STDERR_FILENO, "Vous ne pouvez copier des dossiers qu'en indicant l'option -r");
				free(ph);
				if (dataToCopy != NULL)
					free(dataToCopy);
				return -1;
			}
			else
			{
				res = copyFolder(pathData, pathLocation, name, ph); //copy folder
				if (dataToCopy != NULL)
					free(dataToCopy);
				free(ph);
				return res;
			}
		}
	}

	if (pathLocation->isTarIndicated) //if the copy destination is a tar
	{
		if (pathLocation->isTarBrowsed) //if the copy destination is inside a tar and a path inside the tar has been given
		{
			if (!doesTarExist(pathLocation->path))
			{
				perror("tsh: cp: cpTar: location");
				if (dataToCopy != NULL)
					free(dataToCopy);
				free(ph);
				return -1;
			}
			int z = 0;
			if (pathLocation->nameInTar[strlen(pathLocation->nameInTar) - 1] != '/') //if the copy destination doesn't end with a '/'
				z = 1;																 //set z to one to cat '/' to the copy destination

			char typefile = typeFile(pathLocation->path, pathLocation->nameInTar);
			if (typefile == '5') //if the copy destination (path inside the tar) exist and is a folder
			{
				char *nameFull = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 1 + z);
				strcpy(nameFull, pathLocation->nameInTar);
				if (z)
					strcat(nameFull, "/");
				strcat(nameFull, name);
				//printMessageTsh(1, nameFull);

				res = copyFileInTar(dataToCopy, nameFull, pathLocation->path, ph);
				free(nameFull);
			}
			else if (typefile == '9') //if the copy destination (path inside the tar) does not exist
			{
				if (subFolderExistInTar(pathLocation->path, pathLocation->nameInTar))
				{
					res = copyFileInTar(dataToCopy, pathLocation->nameInTar, pathLocation->path, ph);
				}
				else
				{
					write(STDERR_FILENO, "tsh: cp: Path to copy destination does not exist\n", strlen("tsh: cp: Path to copy destination does not exist\n"));
					return -1;
				}
			}
			else //if the copy destination (path inside the tar) exist but isn't a directory
			{
				deleteFileInTar(pathLocation->nameInTar, pathLocation->path);
				res = copyFileInTar(dataToCopy, pathLocation->nameInTar, pathLocation->path, ph);
			}
		}
		else //if the copy destination is inside a tar and no path inside the tar has been given (ie just copy at the root of the tar, not in a folder and no rename)
		{
			res = copyFileInTar(dataToCopy, name, pathLocation->path, ph);
		}
	}
	else //if the copy location is not a tar but the file to copy is in a tar (line 18 copyData)
	{
		struct stat sb;
		if (stat(pathLocation->path, &sb) != 0) //if no file/directory has been found at the copy destination
		{
			res = cpyDataFileNotInTar(pathLocation->path, dataToCopy, ph); //create a file at the location and copy it
			if (dataToCopy != NULL)
				free(dataToCopy);
			free(ph);
			return res;
		}
		if (S_ISDIR(sb.st_mode)) //if the copy location is in a directory that exist
		{
			res = cpyDataFileNotInTar(concatPathName(pathLocation->path, name), dataToCopy, ph); //add the name of the file to copy to the path for creation
		}
		else //if the copy location is not a directory truncated the copy location
		{
			res = cpyDataFileNotInTar(pathLocation->path, dataToCopy, ph);
		}
	}

	if (dataToCopy != NULL)
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
int copyFolder(pathStruct *pathData, pathStruct *pathLocation, char *name, struct posix_header *ph)
{
	//printMessageTsh(1, "copyfolder");
	//printMessageTsh(1, pathData->path);
	int res, status;
	mode_t modeDir;
	int folder_exist = 1;
	pid_t cpid, wpid;
	if (!pathLocation->isTarIndicated) //if the copy destination is not in a tar
	{
		struct stat sb;
		if (stat(pathLocation->path, &sb) != 0) //if the path to the copy destination doesnt exist
		{
			if (!subFolderExistNotInTar(pathLocation->path))
			{ //if there are no subfolder to where we want to cp
				res = -1;
			}
			else
			{
				res = mkdir(pathLocation->path, S_IRWXU | S_IWGRP | S_IXGRP | S_IXOTH);
			} //create the directory at the copy location
			folder_exist = 0;
		}
		else if (S_ISDIR(sb.st_mode)) //if the cp destination is a folder
		{
			char *pathLocationDir = concatPathName(pathLocation->path, name); //path to the folder to create
			res = mkdir(pathLocationDir, S_IRWXU | S_IWGRP | S_IXGRP | S_IXOTH);
			if (res == -1 && errno == EEXIST) //if folder exist
			{
				cpid = fork(); //create a child process to delete the folder
				if (cpid == -1)
				{
					perror("tsh: cp");
					return -1;
				}
				if (cpid == 0)
				{
					if (execlp("rm", "rm", "-r", pathLocationDir, NULL) == -1) //delete the folder
						perror("tsh: call_existing_command execvp failed");
					exit(EXIT_FAILURE);
				}
				do //wait for child to end
				{
					wpid = waitpid(cpid, &status, WUNTRACED);
				} while (!WIFEXITED(status) && !WIFSIGNALED(status));
				res = mkdir(pathLocationDir, S_IRWXU | S_IWGRP | S_IXGRP | S_IXOTH); //create the folder after the child is dead
			}
			free(pathLocationDir);
		}
		else //if the copy destination is not a folder
		{
			printMessageTsh(STDERR_FILENO, "tsh: cp: Cannot overwrite non-directory with directory");
			return -1;
		}
	}
	else if (pathLocation->isTarBrowsed) //if the copy destination is inside a tar folder (that may or may not need to be created)
	{
		if (!doesTarExist(pathLocation->path))
		{
			res = -1;
		}
		else
		{
			char type = typeFile(pathLocation->path, pathLocation->nameInTar);
			int z = 0;
			if (type == '9')
			{
				if (!subFolderExistInTar(pathLocation->path, pathLocation->nameInTar))
				{
					res = -1;
				}
				else
				{
					res = mkdirInTar(pathLocation->path, pathLocation->nameInTar, ph);
				}
				folder_exist = 0;
			}
			else if (type == '5')
			{
				char *nameDirInTar = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 3);
				strcpy(nameDirInTar, pathLocation->nameInTar);
				if (nameDirInTar[strlen(nameDirInTar) - 1] != '/')
					strcat(nameDirInTar, "/");
				strcat(nameDirInTar, name);
				char typefile = typeFile(pathLocation->path, nameDirInTar);
				if (typefile != '5' && typefile != '9') //if the folder we want to copy already exist in the tar and isn't a folder
				{
					printMessageTsh(STDERR_FILENO, "tsh: cp: Cannot overwrite non-directory with directory");
					free(nameDirInTar);
					return -1;
				}
				if (typefile == '5') //if folder of name nameDir exist at the root of the tar, delete it
				{
					rmWithOptionTar(pathLocation->path, nameDirInTar);
				}
				if (name[strlen(name) - 1] != '/')
					strcat(nameDirInTar, "/");
				res = mkdirInTar(pathLocation->path, nameDirInTar, ph);
				free(nameDirInTar);
			}
			else
			{
				printMessageTsh(STDERR_FILENO, "tsh: cp: Cannot overwrite non-directory with directory");
				return -1;
			}
		}
	}
	else //if the copy destination is inside a tar, at the root (ie not is a folder or no rename)
	{
		if (doesTarExist(pathLocation->path)) //if the tar which is the copy dest exist
		{
			char *nameDir = malloc(strlen(name) + 2);
			strcpy(nameDir, name);
			if (name[strlen(name) - 1] != '/')
				strcat(nameDir, "/");
			char typefile = typeFile(pathLocation->path, nameDir);
			if (typefile != '5' && typefile != '9') //if the folder we want to copy already exist in the tar and isn't a folder
			{
				printMessageTsh(STDERR_FILENO, "tsh: cp: Cannot overwrite non-directory with directory");
				free(nameDir);
				return -1;
			}
			if (typefile == '5') //if folder of name nameDir exist at the root of the tar, delete it
			{
				rmWithOptionTar(pathLocation->path, name);
			}
			res = mkdirInTar(pathLocation->path, nameDir, ph);
			free(nameDir);
		}
		else //if there is no tar at the destination (ie dest folder is just a tar but it doesn't exist)
		{
			res = makeEmptyTar(pathLocation->path);
			folder_exist = 0;
		}
	}

	if (res != -1)
	{
		pathStruct *pathLocationNew = makeNewLocationStruct(pathLocation, name, folder_exist);

		if (pathData->isTarIndicated) //if what we want to copy is inside a tar folder
		{
			char **nameSubFiles = findSubFiles(pathData->path, pathData->nameInTar, 1); //find every subfolder/subfiles of the directory pathData->nameInTar
			int i = 0;
			while (nameSubFiles[i] != NULL) //iterate and call cp
			{
				pathStruct *pathDataNew = malloc(sizeof(pathStruct));
				pathDataNew->isTarBrowsed = 1;
				pathDataNew->isTarIndicated = 1;

				if (pathData->nameInTar != NULL)
				{
					pathDataNew->nameInTar = malloc(strlen(pathData->nameInTar) + strlen(nameSubFiles[i]) + 1);
					strcpy(pathDataNew->nameInTar, pathData->nameInTar);
					strcat(pathDataNew->nameInTar, nameSubFiles[i]);
				}
				else
				{
					pathDataNew->nameInTar = malloc(strlen(nameSubFiles[i]) + 1);
					strcpy(pathDataNew->nameInTar, nameSubFiles[i]);
				}

				//printf("depuis : %s dans %s\n", pathDataNew->nameInTar, pathLocationNew->nameInTar);

				pathDataNew->path = malloc(strlen(pathData->path) + 1);
				strcpy(pathDataNew->path, pathData->path);

				cpTar(pathDataNew, pathLocationNew, 1, nameSubFiles[i]);
				free(nameSubFiles[i]);
				free(pathDataNew->path);
				free(pathDataNew->nameInTar);
				free(pathDataNew);
				i++;
			}
		}
		else //if what we want to copy is not inside a tar
		{
			DIR *dir = opendir(pathData->path);
			struct dirent *dirent;

			while ((dirent = readdir(dir)) != NULL) //iterate over every subfolder/files of the folder pathData->path
			{
				if (strcmp(".", dirent->d_name) != 0 && strcmp("..", dirent->d_name) != 0)
				{
					pathStruct *pathDataNew = malloc(sizeof(pathStruct));
					pathDataNew->isTarBrowsed = 0;
					pathDataNew->isTarIndicated = 0;
					pathDataNew->nameInTar = NULL;
					pathDataNew->path = concatPathName(pathData->path, dirent->d_name);
					//printMessageTsh(1, dirent->d_name);
					//printMessageTsh(1, "end copy folder");
					if (cpTar(pathDataNew, pathLocationNew, 1, dirent->d_name) == -1)
					{
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
	else
	{
		perror("tsh: cp: copyFolder: mkdir");
		return -1;
	}

	return 1;
}

/**
 * @brief Get the data from a file ouside a tar and fills its correspondant posix_header structure
 * 
 * @param path : path to the file we want to copy
 * @param ph : posix_header corresponding to that file
 * @return the data of the file in a char array
 */
char *fileDataNotInTar(char *path, struct posix_header *ph)
{
	struct stat sb;
	if (stat(path, &sb) != 0) //if the file doesn't exist or somthing
	{
		perror("tsh: cp: fileDataNotInTar: stat");
		return NULL;
	}
	int fd;

	if ((fd = open(path, O_RDONLY, S_IRWXU)) == -1)
	{
		perror("tsh: cp: fileDataNotInTar: open");
		return NULL;
	}
	lseek(fd, 0, SEEK_SET);
	remplirHeader(ph, sb);

	int n = 0;
	int k = 0;
	char *data = malloc(sb.st_size);
	read(fd, data, sb.st_size);

	close(fd);
	return data;
}

/**
 * @brief Fills the posix_header corresponding to a file
 * 
 * @param ph : posix_header to be filled
 * @param sb : stat structure of the fiole being copied
 */
void remplirHeader(struct posix_header *ph, struct stat sb)
{
	makePermissions(ph, sb);
	if (S_ISDIR(sb.st_mode))
		sprintf(ph->size, "%011lo", (unsigned long)0);
	else
		sprintf(ph->size, "%011lo", sb.st_size);

	switch (sb.st_mode & S_IFMT)
	{
	case S_IFBLK:
		ph->typeflag = '4';
		break;
	case S_IFCHR:
		ph->typeflag = '3';
		break;
	case S_IFDIR:
		ph->typeflag = '5';
		break;
	case S_IFIFO:
		ph->typeflag = '6';
		break;
	case S_IFLNK:
		ph->typeflag = '1';
		break;
	case S_IFREG:
		ph->typeflag = '0';
		break;
	default:
		printMessageTsh(STDERR_FILENO, "inconnu ?\n");
		break;
	}

	memcpy(ph->magic, TMAGIC, strlen(TMAGIC));
	memcpy(ph->version, TVERSION, strlen(TVERSION));

	sprintf(ph->mtime, "%011lo", sb.st_mtime);
	sprintf(ph->uid, "%07d", sb.st_uid);
	//printf("gid : %d\n", sb.st_gid);
	sprintf(ph->gid, "%07d", sb.st_gid);

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
void makePermissions(struct posix_header *ph, struct stat sb)
{
	int permissions = 0;

	if (sb.st_mode & S_IRUSR)
		permissions += 400;
	if (sb.st_mode & S_IWUSR)
		permissions += 200;
	if (sb.st_mode & S_IXUSR)
		permissions += 100;

	if (sb.st_mode & S_IRGRP)
		permissions += 40;
	if (sb.st_mode & S_IWGRP)
		permissions += 20;
	if (sb.st_mode & S_IXGRP)
		permissions += 10;

	if (sb.st_mode & S_IROTH)
		permissions += 4;
	if (sb.st_mode & S_IWOTH)
		permissions += 2;
	if (sb.st_mode & S_IXOTH)
		permissions += 1;

	sprintf(ph->mode, "%07d", permissions);
}

/**
 * @brief If the path where we want to copy a file is a directory, then this function is used to create the make the copy
 * 
 * @param path path to the file we will create, essentially the path to the directory/name of the copied file
 * @param data data to be copied
 * @param ph posix_header corresponding to that file
 * @return 1 if the copy was successful, -1 if not
 */
int cpyDataFileNotInTar(char *path, char *data, struct posix_header *ph)
{
	int fd;
	if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
	{
		perror("tsh: cp: cpyDataFileNotInTar: open");
		return -1;
	}

	int n;

	n = write(fd, data, octalToDecimal(atoi(ph->size))); //write the content of data buffer into fd

	if (n == -1)
	{
		perror("tsh: cp: cpyDataFileNotInTar: write");
		return -1;
	}
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
char *concatPathBeforeTarPathTar(char **pathBefore, char *name, int op)
{
	char *path = array_to_path(pathBefore, op);
	if (strlen(path) == 0)
		return name;

	return concatPathName(path, name);
}

/**
 * @brief Get the name of the file we want to copy
 * 
 * @param charArray : array of a path, split with the "/"
 * @return the last string in the array, corresponding to the name of the file
 */
char *getLast(char **charArray)
{
	int i = 0;
	while (charArray[i] != NULL)
	{
		i++;
	}
	if (i == 0)
		return NULL;
	else
	{
		char *res = malloc(strlen(charArray[i - 1]) + 1);
		strcpy(res, charArray[i - 1]);
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
pathStruct *makeNewLocationStruct(pathStruct *pathLocation, char *name, int folder_exist)
{
	pathStruct *res = malloc(sizeof(pathStruct));
	res->isTarIndicated = pathLocation->isTarIndicated;
	res->name = NULL;

	if (res->isTarIndicated)
	{
		res->path = malloc(strlen(pathLocation->path) + 1);
		memcpy(res->path, pathLocation->path, strlen(pathLocation->path) + 1);

		char *nameDirInTar;
		res->isTarBrowsed = 1;

		if (pathLocation->isTarBrowsed)
		{
			if (folder_exist)
			{
				nameDirInTar = malloc(strlen(pathLocation->nameInTar) + strlen(name) + 2);
				strcpy(nameDirInTar, pathLocation->nameInTar);
				if (nameDirInTar[strlen(nameDirInTar) - 1] != '/')
					strcat(nameDirInTar, "/");
				strcat(nameDirInTar, name);
			}
			else
			{
				nameDirInTar = malloc(strlen(pathLocation->nameInTar) + 1);
				strcpy(nameDirInTar, pathLocation->nameInTar);
			}
		}
		else
		{
			if (folder_exist)
			{
				nameDirInTar = malloc(strlen(name) + 1);
				strcpy(nameDirInTar, name);
			}
			else
			{
				nameDirInTar = NULL;
				res->isTarBrowsed = 0;
			}
		}

		res->nameInTar = nameDirInTar;
	}

	else
	{
		res->isTarBrowsed = 0;
		res->nameInTar = NULL;
		if (folder_exist)
			res->path = concatPathName(pathLocation->path, name);
		else
		{
			res->path = malloc(strlen(pathLocation->path) + 1);
			memcpy(res->path, pathLocation->path, strlen(pathLocation->path) + 1);
		}
	}

	return res;
}

/**
 * @brief Free a structure pathStruct
 * 
 * @param path 
 */
void freeStruct(pathStruct *pathToFree)
{
	if (pathToFree->nameInTar != NULL)
		free(pathToFree->nameInTar);
	if (pathToFree->path != NULL)
		free(pathToFree->path);
	if (pathToFree->name != NULL)
		free(pathToFree->name);
	free(pathToFree);
}

int subFolderExistNotInTar(char *path)
{
	int nbSlash = 0;
	for (size_t i = 1; i < strlen(path); i++)
	{
		if (path[i] == '/' && i != strlen(path) - 1)
			nbSlash++;
	}
	int index = 0;
	if (nbSlash == 0)
		return 1;

	for (size_t i = 1; i < strlen(path); i++)
	{
		if (path[i] == '/')
		{
			index++;
			if (index == nbSlash)
			{
				char verifyExist[i + 1];
				strncpy(verifyExist, path, i + 1);
				verifyExist[i] = '\0';

				struct stat sb;
				if (stat(verifyExist, &sb) == 0 && S_ISDIR(sb.st_mode))
					return 1;
				return 0;
			}
		}
	}
	return 0;
}

int subFolderExistInTar(char *path_to_tar, char *path_in_tar)
{
	int nbSlash = 0;
	for (size_t i = 1; i < strlen(path_in_tar); i++)
	{
		if (path_in_tar[i] == '/' && i != strlen(path_in_tar) - 1)
			nbSlash++;
	}
	int index = 0;
	if (nbSlash == 0)
		return 1;

	for (size_t i = 1; i < strlen(path_in_tar); i++)
	{
		if (path_in_tar[i] == '/')
		{
			index++;
			if (index == nbSlash)
			{
				char verifyExist[i + 1];
				strncpy(verifyExist, path_in_tar, i + 1);
				verifyExist[i] = '\0';

				if (typeFile(path_to_tar, verifyExist) == '5')
					return 1;
				return 0;
			}
		}
	}
	return 0;
}
