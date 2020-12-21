#include "../headers/cat_tar.h"

/**
 *  @brief command cat on one tar files
 *  @param fd the oppen tar file
 *  @return 1 if the cat worked, and -1 if not
*/
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

/**
 *  @brief 
 *  
*/
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