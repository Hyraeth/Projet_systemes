#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "tar.h"



int ls_tar(char op, char *path, int fd) {
  if(argc == 0 || argc == 1 || argc == 2) printf("Aucun fichier passé en paramètre !\n");
	else{
		char *file = argv[1];
		char *filename = argv[2];
		struct posix_header * header = malloc(sizeof(struct posix_header));
		assert(header);

		int fd = open(file, O_RDONLY);
		if(fd == -1){
	      perror("erreur d'ouverture du fichier");
		  return -1;
	    }

		int n = 0;
		while((n=read(fd, header, BLOCKSIZE))>0){
			if(strcmp(header->name, "\0") == 0){
				printf("Je n'ai pas trouvé %s dans l'archive\n", filename);
				return 0;
			}
			int taille = 0;
			int *ptaille = &taille;
			sscanf(header->size, "%o", ptaille);
			int filesize = ((*ptaille + 512-1)/512);
			if(strcmp((header->name), filename) == 0){
				printf("%s trouvé\n", header->name);
				printf("C'est %s\n", (header->typeflag=='0')?"un fichier standard":(header->typeflag=='5')?"un repertoire":"un type inconnu");
				return 0;
			}
			read(fd, header, BLOCKSIZE*filesize);
		}
		printf("\n");
		close(fd);
	}


  return 0;
}


int main(int argc, char **argv)
{
  return 0;
}