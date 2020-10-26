#include "../headers/cp_tar.h"

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
		char *path = concatPathBeforeTarPathTar(path1[0],path1[1][0],1,"");
		char *pathInTar = array_to_path(path1[2],0);
		dataToCpy = fileDataInTar (pathInTar, path, ph);
		free(path);
		free(pathInTar);
	}

	if (path2[1] == NULL) {
		char *name;
		if (path1[1] == NULL) name = getLast(path1[0]);
		else name = getLast(path1[2]);
		char *path = concatPathBeforeTarPathTar(path2[0],name,1,"");
		if (name == NULL) return -1;
		res = cpyDataFileNotInTar(path,dataToCpy);
		free(path);
	}
	else {
		char *name;
		if (path1[1] == NULL) name = getLast(path1[0]);
		else name = getLast(path1[2]);
		char *path = concatPathBeforeTarPathTar(path2[0],path2[1][0],1,"");
		char *nameInTar = concatPathBeforeTarPathTar(path2[2],name,0,path);
		res = copyFileInTar (dataToCpy,nameInTar,path,ph);
		free(path);
		free(nameInTar);
	}

	free(ph);
	return res;
}


char *fileDataNotInTar (char *path,struct posix_header *ph) {
	int fd;
	if ((fd = open(path,O_RDONLY)) == -1) return NULL;

	struct stat *sb;
	fstat(fd,sb);
	remplirHeader(ph,sb);

	printf("Pourquoi \n");

	char *data = malloc(sb->st_size);
	read(fd,data,sb->st_size);
	close(fd);
	return data;
}

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
    //if (pwd != NULL) memcpy(ph->uname,pwd->pw_name,strlen(pwd->pw_name));
	free(pwd);

	struct group *grp;
	grp = getgrgid(sb->st_gid);
    //if (pwd != NULL) memcpy(ph->gname,grp->gr_name,strlen(grp->gr_name));
	free(grp);
}


int cpyDataFileNotInTar (char * path, char *data) {
	int fd;

	if ((fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1 ) return -1;
	if (write(fd,data,strlen(data)) == -1) return -1;

	return 1;
}

char *concatPathBeforeTarPathTar (char **pathBefore, char *name, int op, char *test) {
	char *path = array_to_path(pathBefore,op);
	if (strlen(path) == 0) return name;

	int len = strlen(path) + strlen(name) + 2;
	if ((path = realloc(path, len)) == NULL) return NULL;
    strcat(path, "/");
    strcat(path, name);
    return path;
}

char *getLast (char **charArray) {
	int i = 0;
	while (charArray[i] != NULL) {
		i++;
	}
	if (i == 0) return NULL;
	else return charArray[i - 1];
}