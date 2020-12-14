#include "../headers/cp_tar.h"
/**
 * @brief General function to make copy file from path1 to directory or tar from path2
 * 
 * @param path1 : path to the file we want to copy 
 * @param path2 : path to the directory or tar where we want to copy
 * @param op 
 * @return 1 if copy was successful, -1 if there was an error 
 */
int cp_tar (char ***path1, char ***path2, int op) {
	char *dataToCpy;
	struct posix_header *ph = malloc(sizeof(struct posix_header));
	int res;

	if (path1[1] == NULL) {
		char *path = array_to_path(path1[0],1);
		dataToCpy = fileDataNotInTar(path,ph);
		free(path);
	}
	else {
		char *path = concatPathBeforeTarPathTar(path1[0],path1[1][0],1);
		char *pathInTar = array_to_path(path1[2],0);
		dataToCpy = fileDataInTar (pathInTar, path, ph);
		free(path);
		free(pathInTar);
	}

	if (path2[1] == NULL) {
		char *name;
		if (path1[1] == NULL) name = getLast(path1[0]);
		else name = getLast(path1[2]);
		char *path = concatPathBeforeTarPathTar(path2[0],name,1);
		if (name == NULL) return -1;
		res = cpyDataFileNotInTar(path,dataToCpy,ph);
		free(path);
	}
	else {
		char *name;
		if (path1[1] == NULL) name = getLast(path1[0]);
		else name = getLast(path1[2]);
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

/**
 * @brief Get the data from a file and fills its correspondant posix_header structure
 * 
 * @param path : path to the file we want to copy
 * @param ph : posix_header corresponding to that file
 * @return the data of the file in a char array
 */
char *fileDataNotInTar (char *path,struct posix_header *ph) {
	int fd;
	if ((fd = open(path,O_RDONLY)) == -1) return NULL;

	struct stat *sb = malloc (sizeof(struct stat));
	fstat(fd,sb);
	remplirHeader(ph,sb);

	char *data = malloc(sb->st_size);
	read(fd,data,sb->st_size);
	close(fd);
	free(sb);
	return data;
}

/**
 * @brief Fills the posix_header corresponding to a file
 * 
 * @param ph : posix_header to be filled
 * @param sb : stat structure of the fiole being copied
 */
void remplirHeader (struct posix_header *ph, struct stat *sb) {
	sprintf(ph->mode,"0000664");
	sprintf(ph->size,"%011lo",sb->st_size);

	switch (sb->st_mode & S_IFMT) {
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

	sprintf(ph->uid,"%07d",sb->st_uid);
	sprintf(ph->gid,"%07d",sb->st_gid);

	struct passwd *pwd;
	pwd = getpwuid(sb->st_uid);
	memcpy(ph->uname, pwd->pw_name, strlen(pwd->pw_name));


	struct group *grp;
	grp = getgrgid(sb->st_gid);
	memcpy(ph->gname, grp->gr_name, strlen(grp->gr_name));

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

	int len = strlen(path) + strlen(name) + 2;
	if ((path = realloc(path, len)) == NULL) return NULL;
    strcat(path, "/");
    strcat(path, name);
    return path;
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