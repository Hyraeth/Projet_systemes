#include "../headers/tar_fun.h"

/**
 * @return 1 if file is a tar, 0 if not 
 */
int isTar(char *file)
{
	if (strlen(file) > 4 && strcmp(file + (strlen(file) - 4), ".tar") == 0)
		return 1;
	return 0;
}

/**
 * @brief Retrieve data from a file in a tar
 * 
 * @param name_file : name of the file in the tar
 * @param path_tar : path to the tar
 * @param ph : posix_header to be filled with header from the file we retrieve
 * @return char* with the data from the file
 */
char *fileDataInTar(char *name_file, char *path_tar, struct posix_header *ph)
{
	int src = open(path_tar, O_RDONLY);
	if (src == -1)
	{
		perror("tsh: cp: fileDataInTar");
		return NULL;
	}
	char name[100];
	char size[12];

	while (read(src, ph, BLOCKSIZE) > 0)
	{

		memcpy(name, ph->name, 100);
		memcpy(size, ph->size, 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		if (strcmpTar(name_file, name))
		{

			char *res = malloc(filesize);
			int n = read(src, res, filesize);

			return res;
		}

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
	}

	return NULL;
}

/**
 * @brief Copy file in a tar
 * 
 * @param dataToCopy : data from the file we want to copy
 * @param name : name of the file to be copied
 * @param path_to_tar : path to the tar where data is to be copied
 * @param ph : posix_header corresponding to the copied file
 * @return 1 if copy was successful, -1 if not 
 */
int copyFileInTar(char *dataToCopy, char *name, char *path_to_tar, struct posix_header *ph)
{
	int fd_dest;
	if ((fd_dest = open(path_to_tar, O_RDWR)) == -1)
	{
		printMessageTsh(STDERR_FILENO, "Erreur lors de l'ouverture du fichier tar d'arrivée");
		return -1;
	}

	int size;
	sscanf(ph->size, "%o", &size);

	char bloc[BLOCKSIZE];
	char sizeInChar[12];
	read(fd_dest, bloc, BLOCKSIZE);

	while (bloc[0] != 0)
	{
		memcpy(sizeInChar, &bloc[124], 12);
		int filesize;
		sscanf(sizeInChar, "%o", &filesize);
		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
		lseek(fd_dest, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(fd_dest, bloc, BLOCKSIZE);
	}

	strcpy(ph->name, name);
	set_checksum(ph);

	lseek(fd_dest, -512, SEEK_CUR);

	write(fd_dest, ph, sizeof(struct posix_header));

	write(fd_dest, dataToCopy, size);

	for (int i = 0; i < 512 - (size % 512); ++i) //On remplit le dernier bloc pour qu'il faisse bien 512o
	{
		write(fd_dest, "\0", 1);
	}

	for (int i = 0; i < 2 * BLOCKSIZE; ++i) //On remplit les deux blocs de 0 à la fin du tar
	{
		write(fd_dest, "\0", 1);
	}

	return 1;
}

int mkdirInTar(char *path_tar, char *path_in_tar, struct posix_header *ph)
{
	char *data = malloc(1);
	data[0] = '\0';

	if (ph == NULL)
	{
		struct posix_header *ph = malloc(sizeof(struct posix_header));
		sprintf(ph->mode, "0000775");
		sprintf(ph->size, "%011lo", strlen(data));
		ph->typeflag = '5';

		memcpy(ph->magic, TMAGIC, strlen(TMAGIC));
		memcpy(ph->version, TVERSION, strlen(TVERSION));

		sprintf(ph->mtime, "%011lo", time(NULL));
		int uid = getuid();
		int gid = getgid();
		sprintf(ph->uid, "%07d", uid);
		sprintf(ph->gid, "%07d", gid);

		struct passwd *pwd;
		pwd = getpwuid(uid);
		memcpy(ph->uname, pwd->pw_name, strlen(pwd->pw_name));

		struct group *grp;
		grp = getgrgid(gid);
		memcpy(ph->gname, grp->gr_name, strlen(grp->gr_name));
		if (path_in_tar[strlen(path_in_tar) - 1] != '/')
		{
			if ((path_in_tar = realloc(path_in_tar, strlen(path_in_tar) + 2)) == NULL)
				perror("tsh: realloc mkdirInTar");
			strcat(path_in_tar, "/");
		}

		int res = copyFileInTar(data, path_in_tar, path_tar, ph);
		free(data);
		return res;
	}
	else
	{
		if (path_in_tar[strlen(path_in_tar) - 1] != '/')
		{
			if ((path_in_tar = realloc(path_in_tar, strlen(path_in_tar) + 2)) == NULL)
				perror("tsh: realloc mkdirInTar");
			strcat(path_in_tar, "/");
		}
		int res = copyFileInTar(data, path_in_tar, path_tar, ph);
		free(data);
		return res;
	}
}

int makeEmptyTar(char *path)
{
	int fd_dest;
	if ((fd_dest = open(path, O_WRONLY | O_CREAT, 0775)) == -1)
	{
		perror("tsh: makeEmptyTar open");
		return -1;
	}

	for (int i = 0; i < 2 * BLOCKSIZE; ++i) //On remplit les deux blocs de 0 à la fin du tar
	{
		write(fd_dest, "\0", 1);
	}
	close(fd_dest);
	return 1;
}

/**
 * @brief Delete a file in a tar
 * 
 * @param name_file : name of the file to be deleted
 * @param path_tar : path of the tar where is located this file
 * @return -1 if there was an error, else return 1 
 */
int deleteFileInTar(char *name_file, char *path_tar)
{
	int src = open(path_tar, O_RDWR);
	if (src == -1)
	{
		perror("tsh: rm");
		close(src);
		return 0;
	}

	char bloc[BLOCKSIZE];
	read(src, bloc, 512);
	char name[100];
	char size[12];

	int sizeToDelete = 0;
	int emplacement = -1;

	while (bloc[0] != 0)
	{

		memcpy(name, bloc, 100);
		memcpy(size, &bloc[124], 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		if (strcmpTar(name_file, name))
		{
			sizeToDelete = 512 + occupiedBlocks * 512;
			emplacement = lseek(src, 0, SEEK_CUR) - 512;
			break;
		}

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(src, bloc, 512);
	}

	if (emplacement != -1)
	{
		int sizeFullTar = lseek(src, 0, SEEK_END);
		lseek(src, emplacement, SEEK_SET);

		for (int i = 0; i < sizeToDelete; i++)
		{
			write(src, '\0', 1);
			lseek(src, 1, SEEK_CUR);
		}

		int sizeToCopy = sizeFullTar - emplacement - sizeToDelete;
		char *dataToMove = malloc(sizeToCopy);

		if (read(src, dataToMove, sizeToCopy) == -1)
		{
			perror("tsh: rm");
			//printMessageTsh(STDERR_FILENO, "Erreur lors de la suppression d'un fichier dans le tar");
			return -1;
		}
		lseek(src, emplacement, SEEK_SET);
		write(src, dataToMove, sizeToCopy);
		ftruncate(src, emplacement + sizeToCopy);

		close(src);
		return 1;
	}

	printMessageTsh(STDERR_FILENO, "tsh: rm: File does not exists");
	close(src);
	return -1;
}

int rmWithOptionTar(char *path_to_tar, char *path_in_tar)
{
	char **subFiles = findSubFiles(path_to_tar, path_in_tar, 0);
	int i = 0;
	while (subFiles[i] != NULL)
	{
		char *fullName = malloc(strlen(path_in_tar) + strlen(subFiles[i]) + 1);
		strcpy(fullName, path_in_tar);
		strcat(fullName, subFiles[i]);
		if (deleteFileInTar(fullName, path_to_tar) == -1)
		{
			while (subFiles[i] != NULL)
			{
				free(subFiles[i]);
				i++;
			}
			free(subFiles);
			return -1;
		}
		free(subFiles[i]);
		i++;
	}
	free(subFiles);
	if (i == 0)
	{
		printMessageTsh(STDERR_FILENO, "tsh: rm: No such directory");
		return -1;
	}
	return 1;
}

int isEmptyDirTar(char *path_to_tar, char *path_in_tar)
{
	char **subFiles = findSubFiles(path_to_tar, path_in_tar, 0);
	int i = 0;
	while (subFiles[i] != NULL)
	{
		i++;
	}

	int res = (i == 1);

	i = 0;
	while (subFiles[i] != NULL)
	{
		free(subFiles[i]);
		i++;
	}
	free(subFiles);
	return res;
}

/**
 * @brief Find all subfiles (depending on the depth) of a given folder in a tar 
 * 
 * @param path_tar : path to the tar
 * @param path_in_tar : path to the folder in the tar
 * @param depth : depth of the files in the folder 
 * (1 equals to directly in the folder, 0 equals to any subfile whether it's directly in the folder or not)
 * @return char** : array with the name of all the files in the folder
 */
char **findSubFiles(char *path_tar, char *path_in_tar, int depth)
{
	int src = open(path_tar, O_RDONLY);
	if (src == -1)
	{
		printMessageTsh(STDERR_FILENO, "Erreur lors de l'ouverture du fichier tar");
		close(src);
		return 0;
	}
	char bloc[BLOCKSIZE];
	read(src, bloc, 512);
	char name[100];
	char size[12];
	int nbSubFiles = 0;

	while (bloc[0] != 0)
	{

		memcpy(name, bloc, 100);
		memcpy(size, &bloc[124], 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
		char *nameCopy;

		if ((nameCopy = isSubFile(path_in_tar, name, depth)) != NULL)
		{
			nbSubFiles++;
			free(nameCopy);
		}

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(src, bloc, 512);
	}

	char **res = malloc((nbSubFiles + 1) * sizeof(char *));
	lseek(src, 0, SEEK_SET);
	read(src, bloc, 512);
	int compteur = 0;

	while (bloc[0] != 0)
	{
		memcpy(name, bloc, 100);
		memcpy(size, &bloc[124], 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
		char *nameCopy;

		if ((nameCopy = isSubFile(path_in_tar, name, depth)) != NULL)
		{
			res[compteur] = nameCopy;
			compteur++;
		}

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(src, bloc, 512);
	}

	res[compteur] = NULL;
	return res;
}

char *isSubFile(char *s, char *toVerify, int depth)
{
	if (strlen(toVerify) <= strlen(s))
		return NULL;
	for (size_t i = 0; i < strlen(s); i++)
	{
		if (s[i] != toVerify[i])
			return NULL;
	}
	int nbSlash = 0;

	if (depth != 0)
	{
		for (size_t i = strlen(s); i < strlen(toVerify); i++)
		{
			if (toVerify[i] == '/' && i != strlen(toVerify) - 1)
			{
				return NULL;
			}
		}
	}

	char *name = malloc(strlen(toVerify) - strlen(s) + 1);
	memcpy(name, &toVerify[strlen(s)], strlen(toVerify) - strlen(s));
	name[strlen(toVerify) - strlen(s)] = '\0';
	return name;
}

/**
 * @brief Return a int to know whether a file is a directory in a tar or not
 * 
 * @param folder : path of the supposed directory
 * @param path : path to the tar in a string array
 * @return 1 if the path is indeed a directory in the tar, else 0 
 */
int isTarFolder(char *folder, char **path)
{
	int i = 1;
	char *path_to_check = malloc(1);
	path_to_check[0] = '\0';
	int len = 1;
	while (path[i] != NULL)
	{
		len += strlen(path[i]) + 1;
		path_to_check = realloc(path_to_check, len);
		strcat(path_to_check, path[i]);
		strcat(path_to_check, "/");
		i++;
	}

	len += strlen(folder) + 1;
	path_to_check = realloc(path_to_check, len);
	strcat(path_to_check, folder);
	strcat(path_to_check, "/");

	if (typeFile(path[0], path_to_check) == '5')
	{
		free(path_to_check);
		return 1;
	}
	else
	{
		free(path_to_check);
		return 0;
	}
}

/**
 * @brief return the type of a file in a tar
 * 
 * @param path_tar : path to the tar
 * @param pathInTar : path to the file in the tar
 * @return a char with the type
 */
char typeFile(char *path_tar, char *pathInTar)
{
	int src = open(path_tar, O_RDONLY);
	if (src == -1)
	{
		perror("tsh: cp: typeFile");
		return 9;
	}
	char bloc[BLOCKSIZE];
	read(src, bloc, 512);
	char name[100];
	char size[12];

	while (bloc[0] != 0)
	{

		memcpy(name, bloc, 100);

		if (strcmpTar(pathInTar, name))
		{
			return bloc[156];
		}

		memcpy(size, &bloc[124], 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(src, bloc, 512);
	}

	return '9';
}

int strcmpTar(char *path_file, char *path_in_tar)
{
	int lenPath1 = strlen(path_file);
	int lenPath2 = strlen(path_in_tar);
	if (lenPath1 < lenPath2 - 1 || lenPath1 > lenPath2)
		return 0;

	int i = 0;
	while (i < lenPath1)
	{
		if (path_file[i] != path_in_tar[i])
			return 0;
		i++;
	}

	return (lenPath1 == lenPath2 || path_in_tar[i] == '/');
}

int octalToDecimal(long int octal)
{
	long int decimal = 0;
	int base = 1;
	long int tmp = octal;

	while (tmp != 0)
	{
		int last_digit = tmp % 10;
		tmp = tmp / 10;
		decimal += last_digit * base;
		base = base * 8;
	}
	return decimal;
}

long int decimalToOctal(long int decimalnum)
{
	long int octalnum = 0, temp = 1;

	while (decimalnum != 0)
	{
		octalnum = octalnum + (decimalnum % 8) * temp;
		decimalnum = decimalnum / 8;
		temp = temp * 10;
	}
	return octalnum;
}

int isEmptyTar(char *path)
{
	int src = open(path, O_RDONLY);
	if (src == -1)
		perror("Read empty tar");

	char bloc[BLOCKSIZE];
	read(src, bloc, 512);

	return (bloc[0] == 0);
}

int doesTarExist(char *path)
{
	struct stat buffer;
	return (stat(path, &buffer) == 0);
}

int renameInTar(char *path_to_tar, char *oldName, char *newName)
{
	int src = open(path_to_tar, O_RDONLY);
	if (src == -1)
		perror("tsh");
	char bloc[BLOCKSIZE];
	read(src, bloc, 512);
	char name[100];
	char size[12];

	while (bloc[0] != 0)
	{

		memcpy(name, bloc, 100);

		if (strcmpTar(oldName, name))
		{
			lseek(src, -BLOCKSIZE, SEEK_CUR);
			write(src, newName, strlen(newName));
			for (size_t i = 0; i < 100 - strlen(newName); i++)
			{
				write(src, "\0", 1);
			}
			return 1;
		}

		memcpy(size, &bloc[124], 12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		lseek(src, BLOCKSIZE * occupiedBlocks, SEEK_CUR);
		read(src, bloc, 512);
	}

	return -1;
}

int isADirectory(pathStruct *pathSrc)
{
	struct stat buffer;
	if (pathSrc->isTarBrowsed)
	{
		return (typeFile(pathSrc->path, pathSrc->nameInTar) == '5');
	}
	if (pathSrc->isTarIndicated)
		return 1;
	if (stat(pathSrc->path, &buffer) != 0)
	{
		return 0;
	}
	else
		return S_ISDIR(buffer.st_mode);
}
