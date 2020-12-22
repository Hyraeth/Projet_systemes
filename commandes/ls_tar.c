#include "../headers/ls_tar.h"

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

//return true if there is juste one time, at the end, c in s
bool contain_one_char(char * s, char c) {
  for (int i=0; i<strlen(s); i++) {
    if (s[i] == c) return (i==strlen(s)-1) ? true : false;
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
  if(strlen(path) != 0) write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 1);
  else write(STDOUT_FILENO, header->name, strlen(header->name));
  print_space(1);
}

//affiche le nom d'un repertoir (avec de la couleur)
void print_name_rep(struct posix_header * header, char * path) {
  write(STDOUT_FILENO, ANSI_COLOR_GREEN"", strlen(ANSI_COLOR_GREEN""));
  if(strlen(path) != 0) write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 2);
  else write(STDOUT_FILENO, header->name, strlen(header->name) - 1);
  write(STDOUT_FILENO, ANSI_COLOR_RESET" ", strlen(ANSI_COLOR_RESET" "));
}

//affiche le type du fichier
void print_type (struct posix_header * header) {
  switch (header->typeflag) {
    case '0' : write(STDOUT_FILENO, "-", 1); break;
    case '5' : write(STDOUT_FILENO, "d", 1); break;
    default : write(STDOUT_FILENO, "i", 1); break;
  }
}
//affiche les droit du FILE
void print_right(struct posix_header * header) {
  for (int i=0; i<3; i++) {
    switch((header->mode + 4)[i]) {
      case '0' : write(STDOUT_FILENO, "---", 3); break;
      case '1' : write(STDOUT_FILENO, "--x", 3); break;
      case '2' : write(STDOUT_FILENO, "-w-", 3); break;
      case '3' : write(STDOUT_FILENO, "-wx", 3); break;
      case '4' : write(STDOUT_FILENO, "r--", 3); break;
      case '5' : write(STDOUT_FILENO, "r-x", 3); break;
      case '6' : write(STDOUT_FILENO, "rw-", 3); break;
      case '7' : write(STDOUT_FILENO, "rwx", 3); break;
    }
  }
}
//affiche le nombre de lien physique
void print_nb_link(struct posix_header * header) {
  
}
//affiche la taille du fichier
void print_size(struct posix_header * header) {
  int taille;
  sscanf(header->size,"%o",&taille);
  char buffer [33];
  if(header->typeflag == '5') taille = 4096;
  sprintf(buffer,"%d",taille);
  print_space(8-strlen(buffer)); //tant pis si la taille depace 8 carractère
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

void print_time(struct posix_header * header) {
  unsigned long taille;
  sscanf(header->mtime,"%lo",&taille);
  time_t timestamp = taille;
  struct tm * pTime = localtime( &timestamp );
  char buffer [65];
  strftime(buffer, 65, "%b %d %H:%M", pTime );
  print_space(8-strlen(buffer)); //tant pis si la taille depace 8 carractère
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

//affiche toute les info supplementaire de la commande "ls -l" autre que le nom
void print_ls_l (struct posix_header * header) {
  print_type(header); //affiche le type du fichier
  print_right(header); //affiche les droits
  print_space(1);
  print_nb_link(header);
  print_space(1);
  write(STDOUT_FILENO, header->uname, strlen(header->uname)); //affiche le nom du propriétaire
  print_space(1);
  write(STDOUT_FILENO, header->gname, strlen(header->gname)); //affiche le nom du groupe
  print_space(1);
  print_size(header); //affiche la taille du fichier
  print_space(1);
  print_time(header); //affiche l'horodatage
  print_space(1);
}

//affiche ce qu'affiche ls sur un FILE en fonction de la commande
void print_header_name (char *op, struct posix_header * header, char * path) {
  if (contain_one_char(header->name + strlen(path) + 1, '/')) {
    if(op == NULL) {
      print_name_rep(header,path);
    }
    else if (strcmp(op,"-l") == 0){
      print_ls_l(header);
      print_name_rep(header,path);
      write(STDOUT_FILENO, "\n", strlen("\n"));
    }
    else
      print_name_rep(header,path);
  }
  else if (!contain_char(header->name + strlen(path) + 1, '/') && !(strcmp(header->name + strlen(path) + 1, "\0") == 0)) {
    if(op == NULL) {
      print_name_file(header,path);
    }
    else if (strcmp(op,"-l") == 0){
      print_ls_l(header);
      print_name_file(header,path);
      write(STDOUT_FILENO, "\n", strlen("\n"));
    }
    else
      print_name_file(header,path);
  }
}

//fonction ls with option -l or not
int ls_tar(char *op, char *path, int fd) {
  struct posix_header * header = malloc(sizeof(struct posix_header));
  assert(header);
  if(fd == -1){
    perror("erreur d'ouverture du fichier");
    return -1;
  }
  int n = 0;
  while((n=read(fd, header, BLOCKSIZE))>0){
    if (s_is_in_string(header->name, path)) {
      print_header_name(op,header,path);
    }
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + 512-1)/512);
    lseek(fd, BLOCKSIZE*filesize, SEEK_CUR);
  }
  write(STDOUT_FILENO, "\n", strlen("\n"));
  lseek(fd, 0, SEEK_SET);
  free(header);
  return 1;
}
/*
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
*/