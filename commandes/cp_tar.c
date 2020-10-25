#include "../headers/cp_tar.h"
#include "../headers/tsh_fun.h"
#include "../headers/tar_fun.h"

int cp_tar (char ***path1, char ***path2, int op) {
	char *dataToCpy;
	if (path1[1] == NULL) {
		char *path = array_to_path(path1[0],1);
		dataToCpy = fileDataNotInTar(path);
		free(path);
	}
	else {
		char *path = concatPathBeforeTarPathTar(path1[0],path1[1][0],1,"");
		char *pathInTar = array_to_path(path1[2],0);
		dataToCpy = fileDataInTar (pathInTar, path);
		free(path);
		free(pathInTar);
	}

	if (path2[1] == NULL) {
		char *name = getLast(path1[0]);
		char *path = concatPathBeforeTarPathTar(path2[0],name,1,"");
		if (name == NULL) return -1;
		if (cpyDataFileNotInTar(path,dataToCpy) == -1) return -1;
		free(path);
	}
	else {
		char *name;
		if (path1[1] == NULL) name = getLast(path1[0]);
		else name = getLast(path1[2]);
		char *path = concatPathBeforeTarPathTar(path2[0],path2[1][0],1,"");
		char *nameInTar = concatPathBeforeTarPathTar(path2[2],name,0,path);
		if (copyFileInTar (dataToCpy,nameInTar,path) == -1) return -1;
		free(path);
		free(nameInTar);
	}

	return 1;
}


char *fileDataNotInTar (char *path) {
	int fd;
	if ((fd = open(path,O_RDONLY)) == -1) return NULL;

	int size = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);

	char *data = malloc(size + 1);
	read(fd,data,size);
	return data;
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