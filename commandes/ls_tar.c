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

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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

void print_space(int n){
  for (int i=0; i<n; i++) {
    write(STDOUT_FILENO, " ", strlen(" "));
  }
}

//affiche le nom d'un fichier
void print_name_file(struct posix_header * header, char * path) {
  write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 1);
  print_space(1);
}

//affiche le nom d'un repertoir (avec de la couleur)
void print_name_rep(struct posix_header * header, char * path) {
  write(STDOUT_FILENO, ANSI_COLOR_GREEN"", strlen(ANSI_COLOR_GREEN""));
  write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 2);
  write(STDOUT_FILENO, ANSI_COLOR_RESET" ", strlen(ANSI_COLOR_RESET" "));
}

//affiche les droit du FILE
void print_right(struct posix_header * header) {
  for (int i=0; i<3; i++) {
    switch((header->mode + 4)[i]) {
      case '0' : write(STDOUT_FILENO, "---", strlen("---")); break;
      case '1' : write(STDOUT_FILENO, "--x", strlen("--x")); break;
      case '2' : write(STDOUT_FILENO, "-w-", strlen("-w-")); break;
      case '3' : write(STDOUT_FILENO, "-wx", strlen("-wx")); break;
      case '4' : write(STDOUT_FILENO, "r--", strlen("r--")); break;
      case '5' : write(STDOUT_FILENO, "r-x", strlen("r-x")); break;
      case '6' : write(STDOUT_FILENO, "rw-", strlen("rw-")); break;
      case '7' : write(STDOUT_FILENO, "rwx", strlen("rwx")); break;
    }
  }
  print_space(1);
}
//affiche la taille du fichier
void print_size(struct posix_header * header) {
  int taille;
  sscanf(header->size,"%o",&taille);
  char buffer [33];
  sprintf(buffer,"%d",taille);
  print_space(8-strlen(buffer)); //tant pis si la taille depace 8 carractère
  write(STDOUT_FILENO, buffer, strlen(buffer));
}
//affiche toute les info supplementaire de la commande "ls -l" autre que le nom
void print_ls_l (struct posix_header * header) {
  write(STDOUT_FILENO, "-", 1);
  print_right(header); //affiche les droits
  write(STDOUT_FILENO, header->name + 156, 1); //affiche le type
  print_space(1);
  write(STDOUT_FILENO, header->uname, strlen(header->uname)); //affiche le nom du propriétaire
  print_space(1);
  write(STDOUT_FILENO, header->gname, strlen(header->gname)); //affiche le nom du groupe
  print_space(1);
  print_size(header); //affiche la taille du fichier
  print_space(1);
  write(STDOUT_FILENO, header->mtime, strlen(header->mtime)); //affiche l'horodatage
  print_space(1);
}

//affiche ce qu'affiche ls sur un FILE en fonction de la commande
void print_header_name(char op, struct posix_header * header, char * path) {
  if (contain_one_char(header->name + strlen(path) + 1, '/')) {
    if (op == 'l'){
      print_ls_l(header);
      print_name_rep(header,path);
      write(STDOUT_FILENO, "\n", strlen("\n"));
    }
    else
      print_name_rep(header,path);
  }
  else if (!contain_char(header->name + strlen(path) + 1, '/') && !(strcmp(header->name + strlen(path) + 1, "\0") == 0)) {
    if (op == 'l'){
      print_ls_l(header);
      print_name_file(header,path);
      write(STDOUT_FILENO, "\n", strlen("\n"));
    }
    else
      print_name_file(header,path);
  }
}

//fonction ls with option -l or not
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
      print_header_name(op,header,path);
    }
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + 512-1)/512);
    read(fd, header, BLOCKSIZE*filesize);
  }
  write(STDOUT_FILENO, "\n", strlen("\n"));
  return 0;
}

int main(int argc, char *argv[]){
  if(argc <= 2) printf("Pas de fichier\n");
  else {
    int fd = open(argv[1], O_RDONLY);
    ls_tar('0', argv[2], fd);
    close(fd);
    write(STDOUT_FILENO, "\n", strlen("\n"));
    fd = open(argv[1], O_RDONLY);
    ls_tar('l', argv[2], fd);
    close(fd);
  }
}
