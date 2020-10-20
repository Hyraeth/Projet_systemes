#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "tar.h"

//return true if s is in string at the begining
bool is_in_string(char * string, char * s) {
  for (int i=0; i<strlen(s); i++) {
    if (string[i] != s[i])
      return false;
  }
  return true;
}

void print_header_name(struct posix_header * header, char * path) {
  write(STDOUT_FILENO, header->name + strlen(path) + 1, strlen(header->name) - strlen(path) - 1);
  write(STDOUT_FILENO, "  ", strlen("  "));
}

int ls_tar(char op, char *path, int fd) {
  struct posix_header * header = malloc(sizeof(struct posix_header));
  assert(header);
  if(fd == -1){
    perror("erreur d'ouverture du fichier");
    return -1;
  }

  int n = 0;
  while((n=read(fd, header, BLOCKSIZE))>0){
    int i = 0;
    if (is_in_string(header->name, path)) {
      print_header_name(header,path);
    }

    /*char d[] = "/";
    char *ppath = strtok(path,d);
    char *pheader = strtok(header->name,d);
    int i = 0;
    while (pheader!=NULL) {
      write(STDOUT_FILENO, pheader, strlen(pheader));
      write(STDOUT_FILENO, ppath, strlen(ppath));
      if (ppath == NULL) {
        pheader = strtok(NULL,d);
        write(STDOUT_FILENO, pheader, strlen(pheader));
      }
      else if (strcmp(ppath, pheader) != 0) exit(0);
      ppath = strtok(NULL,d);
      pheader = strtok(NULL,d);
    }
    print_header_name(header);*/



    /*

    if(strcmp(header->name, path) == 0){
      write(STDOUT_FILENO, path, strlen(path));
      write(STDOUT_FILENO, " pas find dans l'archive\n", strlen(" pas trouver dans l'archive\n"));
      //printf("Je n'ai pas trouvé %s dans l'archive\n", path);
      return 0;
    }*/
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + 512-1)/512);
    /*if(strcmp((header->name), path) == 0){
      write(STDOUT_FILENO, header->name, strlen(header->name));
      //printf("%s trouvé\n", header->name);
      //printf("C'est %s\n", (header->typeflag=='0')?"un fichier standard":(header->typeflag=='5')?"un repertoire":"un type inconnu");
      return 0;
    }
    
    */



    read(fd, header, BLOCKSIZE*filesize);
  }
  printf("\n");
  return 0;
}

int main(int argc, char *argv[]){
  if(argc <= 2) printf("Pas de fichier\n");
  else {
    int fd = open(argv[1], O_RDONLY);
    ls_tar('0', argv[2], fd);
    close(fd);
  }
}
