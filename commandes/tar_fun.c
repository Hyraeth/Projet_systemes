#include "tar_fun.h"
#include "tar.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BLOCKSIZE 512
#define BLOCKBITS 9

int isTar(char *file) {
    if(strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
    return 0;
}

char *fileDataInTar (char *name_file, char *path_tar) {
	int src = open(path_tar,O_RDONLY);
	if (src == -1) perror("tsh");
	char bloc[BLOCKSIZE];
	read(src,bloc,512);
	char name[100];
	char size[12];


	while (bloc[0] != 0) {

		memcpy(name,bloc,100);
		printf("%s\n", name);
		memcpy(size,&bloc[124],12);
		int filesize;
		sscanf(size,"%o",&filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		if (strcmp(name,name_file) == 0){
			char *res = malloc(BLOCKSIZE * occupiedBlocks + 1);
			int n = read(src,res,BLOCKSIZE * occupiedBlocks);
			res[n] = 0;
			return res;
		}

		lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
		read(src,bloc,512);
	}

	return NULL;
}

int copyFileInTar (int fd_src, const char *name, int fd_dest) {
	int size = lseek(fd_src,0,SEEK_END);
	lseek(fd_src,0,SEEK_SET);

	struct posix_header *ph = malloc(sizeof(struct posix_header));
	memcpy(ph->name, name, strlen(name));
	sprintf(ph->mode,"0000700");
	sprintf(ph->size,"%011o",size);
	ph->typeflag = '0';
	memcpy(ph->magic, TMAGIC, strlen(TMAGIC));
	memcpy(ph->version, TVERSION, strlen(TVERSION));

	set_checksum(ph);

	write(fd_dest,ph,sizeof(struct posix_header));

	int occupiedBlocks = (size + BLOCKSIZE - 1) >> BLOCKBITS;

	char buf[BLOCKSIZE];
	for (int i = 0; i < occupiedBlocks; ++i)
	{
		int buflen = read(fd_src,buf,512);
		write(fd_dest, buf, buflen);
	}

	for (int i = 0; i < 512 - (size%512); ++i) //On remplit le dernier bloc pour qu'il faisse bien 512o
	{
		write(fd_dest,"0",1);
	}

	return 1;
}

int deleteFileInTar (char *name_file, char *path_tar) {
    int src = open(path_tar,O_RDWR);
    if (src == -1) {
        perror("tsh");
        close(src);
        return 0;
    }
    char bloc[BLOCKSIZE];
    read(src,bloc,512);
    char name[100];
    char size[12];

    int sizeToDelete = 0;
    int emplacement = -1;

    while (bloc[0] != 0) {

        memcpy(name,bloc,100);
        printf("%s\n", name);
        memcpy(size,&bloc[124],12);
        int filesize;
        sscanf(size,"%o",&filesize);

        int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

        if (strcmp(name,name_file) == 0){
            sizeToDelete = 512 + occupiedBlocks*512;
            emplacement = lseek(src,0,SEEK_CUR) - 512;
            break;
        }

        lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
        read(src,bloc,512);
    }

    if (emplacement != -1) {
        int sizeFullTar = lseek(src,0,SEEK_END);
        lseek(src,emplacement,SEEK_SET);
        for (int i = 0; i < sizeToDelete; i++)
        {
        	write(src,"0",1);
        }
        int sizeToCopy = sizeFullTar - emplacement - sizeToDelete;
        char dataToMove[sizeToCopy];
        if (read(src,dataToMove,sizeToCopy) == -1 ) perror("tsh");
        lseek(src,emplacement,SEEK_SET);
        write(src,dataToMove,sizeToCopy);
        ftruncate(src,emplacement+sizeToCopy);

        printf("%d\n", sizeFullTar);
        printf("%d\n", sizeToCopy);
        close(src);
        return 1;
    }

    perror("Le fichier à supprimer n'existe pas");
    close(src);
    return 0;
}


char *path_strstr (char *path) {
	printf("%s\n",path );
	char *res = strstr(path,".tar");

	if (res == NULL) return NULL;
   	
   	if (res != NULL && res[4] != 0 && res[4] != '/') {
   		int pos = strlen(path) - strlen(res) + 4;
   		return path_strstr(&path[pos]);
   	}
   	return res;
}

int isTarFolder (char *folder, char**path){
	int i = 1;
	char *path_to_check = malloc(1);
	int len = 1;
	while (path[i] != NULL) {
		len += strlen(path[i]) + 1;
		path_to_check = realloc (path_to_check,len);
		strcat(path_to_check,path[i]);
		strcat(path_to_check,"/");
		i++;
	}

	len += strlen(folder) + 1;
	path_to_check = realloc (path_to_check,len);
	strcat(path_to_check,folder);
	strcat(path_to_check,"/");

	if (typeFile(path[0],path_to_check) == '5') return 1;
	else return 0;
}


char typeFile (char *path_tar, char *pathInTar) {
	int src = open(path_tar,O_RDONLY);
	if (src == -1) perror("tsh");
	char bloc[BLOCKSIZE];
	read(src,bloc,512);
	char name[100];
	char size[12];


	while (bloc[0] != 0) {

		memcpy(name,bloc,100);

		if (strcmp(name,pathInTar) == 0){
			return bloc[156];
		}

		memcpy(size,&bloc[124],12);
		int filesize;
		sscanf(size,"%o",&filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
		read(src,bloc,512);
	}

	return '9';
}

/**int main(int argc, char const *argv[])
{
	char **res = path_to_tar_file_path(argv[1]);
	printf("%s et %s \n",res[0],res[1] );

	printf("%s\n", fileDataInTar("supp.txt","toto.tar"));

	char *A[] = {"titi.tar", "toto",NULL};


	if (isTarFolder("tat",A)) printf("BIEN\n");
	else printf("PAS BIEN\n");

	printf(" Resultat %d\n", deleteFileInTar("supp.txt","toto.tar"));

	return 0;
}**/

