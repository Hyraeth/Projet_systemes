#include "../headers/cat_tar.h"

/**
 * @brief 
 * 
 * @param header 
 * @param path 
 * @return int 
 */
int cat_tar(struct posix_header * header, char * path) {
}

/**
 *  @brief command cat on one tar file
 *
 *  @param fd the oppen tar file
 *  @return 1 if the cat worked, and -1 if not
 */
int cat(char *path, int fd) {
  struct posix_header * header = malloc(sizeof(struct posix_header));
  assert(header);
  if(fd == -1){
    perror("erreur d'ouverture du fichier");
    return -1;
  }
  int n = 0;
  while((n=read(fd, header, BLOCKSIZE))>0){
    if (s_is_in_string(header->name, path)) {
      cat_tar(op,header,path);
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

/**
 *  @brief test for the cat_tar
 *  
 */
int main(int argc, char *argv[]){
  if(argc <= 1) printf("Pas de fichier\n");
  else {
    int fd = open(argv[1], O_RDONLY);
    cat_tar(fd);
    close(fd);
    write(STDOUT_FILENO, "\n", strlen("\n"));
  }
}