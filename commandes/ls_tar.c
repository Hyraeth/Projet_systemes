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
bool s_is_in_string(char * string, char * s) {
  for (int i=0; i<strlen(s); i++) {
    if (string[i] != s[i])
      return false;
  }
  return true;
}

//retun true if c is in s
bool contain_char(char * s, char c) {
  for (int i=0; i<strlen(s); i++) {
    if (s[i] == c)
      return true;
  }
  return false;
}

//return true if there is juste one time c in s
bool contain_one_char(char * s, char c) {
  for (int i=0; i<strlen(s); i++) {
    if (s[i] == c)
      if (i==strlen(s)-1)
        return true;
      else
        return false;
  }
  return false;
}

void print_header_name(struct posix_header * header, char * path) {
  if (contain_one_char(header->name + strlen(path) + 1, '/') || !contain_char(header->name + strlen(path) + 1, '/')) {
    write(STDOUT_FILENO, header->name + strlen(path) + 1, strlen(header->name) - strlen(path) - 1);
    write(STDOUT_FILENO, " ", strlen(" "));
  }
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
    if (s_is_in_string(header->name, path)) {
      print_header_name(header,path);
    }
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + 512-1)/512);
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
