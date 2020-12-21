#include "../headers/cat_tar.h"

int cat_tar(int fd) {
  struct posix_header * header = malloc(sizeof(struct posix_header));
  assert(header);
  if(fd == -1){
    perror("erreur d'ouverture du fichier");
    return -1;
  }
  write(STDOUT_FILENO, "\n", strlen("\n"));
  lseek(fd, 0, SEEK_SET);
  free(header);
  return 1;
}

int main(int argc, char *argv[]){
}