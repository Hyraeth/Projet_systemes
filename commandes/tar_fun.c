#include "../headers/tar_fun.h"

int isTar(char *file) {
    if(strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
    return 0;
}

char *fileDataInTar (char *name_file, char *path_tar, struct posix_header *ph) {
	int src = open(path_tar,O_RDONLY);
	if (src == -1) perror("tsh");
	char name[100];
	char size[12];


	while (read(src,ph,BLOCKSIZE) > 0) {

		memcpy(name,ph->name,100);
		memcpy(size,ph->size,12);
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
	}

	return NULL;
}

int copyFileInTar (char *dataToCopy, char *name, char *path_to_tar, struct posix_header *ph) {
	int fd_dest;


	if ((fd_dest = open(path_to_tar,O_RDWR)) == -1) return -1;

	int size = strlen(dataToCopy);
	lseek(fd_dest,-2*BLOCKSIZE,SEEK_END);

	memcpy(ph->name, name, strlen(name));

	write(fd_dest,ph,sizeof(struct posix_header));

	write(fd_dest,dataToCopy,size);


	for (int i = 0; i < 512 - (size%512); ++i) //On remplit le dernier bloc pour qu'il faisse bien 512o
	{
		write(fd_dest,'\0',1);
	}

	lseek(fd_dest,0,SEEK_END);

	for (int i = 0; i < 2 * BLOCKSIZE; ++i) //On remplit les deux blocs de 0 à la fin du tar
	{
		write(fd_dest,'\0',1);
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

        printf("%d\n",sizeToDelete );
        for (int i = 0; i < sizeToDelete; i++)
        {
        	write(src,'\0',1);
        	lseek(src,1,SEEK_CUR);
        }
        int sizeToCopy = sizeFullTar - emplacement - sizeToDelete;
        char dataToMove[sizeToCopy];
        if (read(src,dataToMove,sizeToCopy) == -1 ) perror("tsh");
        lseek(src,emplacement,SEEK_SET);
        write(src,dataToMove,sizeToCopy);
        ftruncate(src,emplacement+sizeToCopy);

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

/*

int main(int argc, char const *argv[])
{
	char **res = path_to_tar_file_path(argv[1]);
	printf("%s et %s \n",res[0],res[1] );

	printf("%s\n", fileDataInTar("supp.txt","toto.tar"));

	char *A[] = {"titi.tar", "toto",NULL};


	if (isTarFolder("tat",A)) printf("BIEN\n");
	else printf("PAS BIEN\n");

	printf(" Resultat %d\n", deleteFileInTar("supp.txt","toto.tar"));

	return 0;
}*/

