#include "../headers/tar_fun.h"

/**
 * @return 1 if file is a tar, 0 if not 
 */
int isTar(char *file) {
    if(strlen(file) > 4 && strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
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
char *fileDataInTar (char *name_file, char *path_tar, struct posix_header *ph) {
	int src = open(path_tar,O_RDONLY);
	if (src == -1) perror("tsh");
	char name[100];
	char size[12];

	while (read(src,ph,BLOCKSIZE) > 0) {

		memcpy(name,ph->name,100);
		memcpy(size,ph->size,12);
		int filesize;
		sscanf(size, "%o", &filesize);

		int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;

		if (strcmpTar(name_file,name)){

			char *res = malloc(filesize);
			int n = read(src,res,filesize);

			return res;
		}

		lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
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
int copyFileInTar (char *dataToCopy, char *name, char *path_to_tar, struct posix_header *ph) {
	int fd_dest;
	if ((fd_dest = open(path_to_tar,O_RDWR)) == -1) {
		printMessageTsh("Erreur lors de l'ouverture du fichier tar d'arrivée");
		return -1;
	}

	int size = strlen(dataToCopy);
	char bloc[BLOCKSIZE];
	do
	{
		read(fd_dest,bloc,512);
	} while (bloc[0] != 0);

	strcpy(ph->name,name);
	set_checksum(ph);

	lseek(fd_dest,-512,SEEK_CUR);

	write(fd_dest,ph,sizeof(struct posix_header));

	write(fd_dest,dataToCopy,size);

	for (int i = 0; i < 512 - (size%512); ++i) //On remplit le dernier bloc pour qu'il faisse bien 512o
	{
		write(fd_dest,"\0",1);
	}

	for (int i = 0; i < 2 * BLOCKSIZE; ++i) //On remplit les deux blocs de 0 à la fin du tar
	{
		write(fd_dest,"\0",1);
	}

	return 1;
}

/**
 * @brief Delete a file in a tar
 * 
 * @param name_file : name of the file to be deleted
 * @param path_tar : path of the tar where is located this file
 * @return -1 if there was an error, else return 1 
 */
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

        if (strcmpTar(name_file,name)){
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
    return -1;
}

char **findSubFiles (char *path_tar, char *path_in_tar) {
	int src = open(path_tar,O_RDONLY);
    if (src == -1) {
        perror("tsh");
        close(src);
        return 0;
    }
    char bloc[BLOCKSIZE];
    read(src,bloc,512);
    char name[100];
    char size[12];
	int nbSubFiles = 0;

    while (bloc[0] != 0) {

        memcpy(name,bloc,100);
        memcpy(size,&bloc[124],12);
        int filesize;
        sscanf(size,"%o",&filesize);

        int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
		char *nameCopy;

        if ((nameCopy = isSubFile(path_in_tar,name)) != NULL){
            nbSubFiles ++;
			free(nameCopy);
        }

        lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
        read(src,bloc,512);
    }

	char **res = malloc((nbSubFiles + 1) * sizeof(char *));
	lseek(src,0,SEEK_SET);
	read(src,bloc,512);
	int compteur = 0;

	while (bloc[0] != 0) {
        memcpy(name,bloc,100);
        memcpy(size,&bloc[124],12);
        int filesize;
        sscanf(size,"%o",&filesize);

        int occupiedBlocks = (filesize + BLOCKSIZE - 1) >> BLOCKBITS;
		char *nameCopy;

        if ((nameCopy = isSubFile(path_in_tar,name)) != NULL){
            res[compteur] = nameCopy;
			compteur++;
        }

        lseek(src,BLOCKSIZE*occupiedBlocks,SEEK_CUR);
        read(src,bloc,512);
    }

	res[compteur] = NULL;
	return res;
}

char *isSubFile (char *s, char *toVerify) {
	if (strlen(toVerify) <= strlen(s)) return NULL;
	for (size_t i = 0; i < strlen(s); i++)
	{
		if (s[i] != toVerify[i]) return NULL;
	}
	int nbSlash = 0;
	for (size_t i = strlen(s); i < strlen(toVerify); i++)
	{
		if (toVerify[i] == '/') {
			nbSlash += 1;
			if (nbSlash > 1) return NULL;
		}
	}

	char *name = malloc(strlen(toVerify) - strlen(s) + 1);
	memcpy(name,&toVerify[strlen(s)],strlen(toVerify) - strlen(s));
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
int isTarFolder (char *folder, char**path){
	int i = 1;
	char *path_to_check = malloc(1);
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
		return 1;
	else
		return 0;
}

/**
 * @brief return the type of a file in a tar
 * 
 * @param path_tar : path to the tar
 * @param pathInTar : path to the file in the tar
 * @return a char with the type
 */
char typeFile (char *path_tar, char *pathInTar) {
	int src = open(path_tar,O_RDONLY);
	if (src == -1) perror("tsh");
	char bloc[BLOCKSIZE];
	read(src, bloc, 512);
	char name[100];
	char size[12];

	while (bloc[0] != 0)
	{

		memcpy(name, bloc, 100);

		if (strcmpTar(pathInTar,name)){
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

int strcmpTar (char *path_file, char *path_in_tar) {
	int lenPath1 = strlen(path_file);
	int lenPath2 = strlen(path_in_tar);
	if (lenPath1 < lenPath2 - 1 || lenPath1 > lenPath2) return 0;

	int i = 0;
	while (i < lenPath1)
	{
		if (path_file[i] != path_in_tar[i]) return 0;
		i++;
	}
	
	return (lenPath1 == lenPath2 || path_in_tar[i] == '/');
}

int octalToDecimal (long int octal) {
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

