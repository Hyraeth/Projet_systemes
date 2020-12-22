#include "../headers/cat_tar.h"
#include "../commandes/tar_fun.c"
/**
 * @brief copies the contents of a file
 * 
 * @param header the target file header
 * @param path the path of the tar file to the target file
 * @return int 
 */
int cat_tar(struct posix_header *header, int fd)
{
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + BLOCKSIZE - 1) / BLOCKSIZE);
    //block
    char *block = malloc(sizeof(char) * BLOCKSIZE * filesize);
    read(fd, block, BLOCKSIZE);
    write(STDOUT_FILENO, block, BLOCKSIZE);
}

/**
 *  @brief command cat on one file in a tar
 *
 *  @param path_tar the path to the tar file
 *  @param path the path of the tar file to the target file
 *  @param fd the oppen tar file
 *  @return 1 if the cat worked, and -1 if not
 */
int cat(char *path_tar, char *path)
{
    struct posix_header *header = malloc(sizeof(struct posix_header));
    assert(header);
    if (typeFile(path_tar, path) != '0')
    {
        perror("cible non acceptée");
        return -1;
    }
    int fd = open(path_tar, O_RDONLY);
    if (fd == -1)
    {
        perror("erreur d'ouverture du fichier");
        close(fd);
        return -1;
    }
    int n = 0;
    while ((n = read(fd, header, BLOCKSIZE)) > 0)
    {
        if (strcmp(header->name, path) == 0)
        {
            cat_tar(header, fd);
        }
        int taille = 0;
        int *ptaille = &taille;
        sscanf(header->size, "%o", ptaille);
        int filesize = ((*ptaille + 512 - 1) / 512);
        lseek(fd, BLOCKSIZE * filesize, SEEK_CUR);
    }
    lseek(fd, 0, SEEK_SET);
    free(header);
    close(fd);
    return 1;
}

/*
int main(int argc, char *argv[]){
  if(argc <= 1) printf("Pas de fichier\n");
  else {
    cat(argv[1], argv[2]);
    write(STDOUT_FILENO, "\n", strlen("\n"));
  }
}*/