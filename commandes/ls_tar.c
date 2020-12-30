#include "../headers/ls_tar.h"

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

/**
 * @brief check if a character chain contain an other character chain, at the beginning
 * 
 * @return true if s is in string
 * @return false s isn't in string
 */
bool s_is_in_string(char *string, char *s)
{
  if (strlen(s) > strlen(string))
    return false;
  for (int i = 0; i < strlen(s); i++)
  {
    if (string[i] != s[i])
      return false;
  }
  return true;
}

int isInFolder(char *s, char *toVerify)
{
  if (strlen(toVerify) <= strlen(s))
    return 0;

  for (size_t i = 0; i < strlen(s); i++)
  {
    if (s[i] != toVerify[i])
      return 0;
  }
  int beginIndex = strlen(s);
  if (beginIndex != 0)
  {
    if (s[beginIndex - 1] != '/')
    {
      if (toVerify[beginIndex] != '/')
        return 0;
      else
      {
        if (strlen(toVerify) == beginIndex + 1)
          return 0;
        beginIndex++;
      }
    }
    else if (beginIndex == strlen(toVerify))
      return 0;
  }
  int nbSlash = 0;
  for (size_t i = beginIndex; i < strlen(toVerify); i++)
  {
    if (toVerify[i] == '/')
    {
      if (i != strlen(toVerify) - 1)
        return 0;
    }
  }
  return 1;
}

/**
 * @brief check if a character is in a character chain
 * 
 * @param s the character chain
 * @param c the char
 * @return true if c is in s
 * @return false if c is not in s
 */
bool contain_char(char *s, char c)
{
  for (int i = 0; i < strlen(s); i++)
  {
    if (s[i] == c)
      return true;
  }
  return false;
}

/**
 * @brief check if there is juste one time, and at the end, a character in a character chain
 * 
 * @param s the character chain
 * @param c the character
 * @return true if c is at the end
 * @return false if it isn't
 */
bool contain_one_char(char *s, char c)
{
  for (int i = 0; i < strlen(s); i++)
  {
    if (s[i] == c)
      return (i == strlen(s) - 1) ? true : false;
  }
  return false;
}

/**
 * @brief print some space
 * 
 * @param n number of space
 */
void print_space(int n)
{
  for (int i = 0; i < n; i++)
  {
    write(STDOUT_FILENO, " ", strlen(" "));
  }
}

/**
 * @brief print the name of a file
 * 
 */
void print_name_file(struct posix_header *header, char *path)
{
  if (strlen(path) != 0)
    write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 1);
  else
    write(STDOUT_FILENO, header->name, strlen(header->name));
  print_space(1);
}

//affiche le nom d'un repertoir (avec de la couleur)
/**
 * @brief print the name of a repertoire
 * 
 */
void print_name_rep(struct posix_header *header, char *path)
{
  write(STDOUT_FILENO, ANSI_COLOR_GREEN "", strlen(ANSI_COLOR_GREEN ""));
  if (strlen(path) != 0)
    write(STDOUT_FILENO, (header->name + strlen(path) + 1), strlen(header->name) - strlen(path) - 2);
  else
    write(STDOUT_FILENO, header->name, strlen(header->name) - 1);
  write(STDOUT_FILENO, ANSI_COLOR_RESET " ", strlen(ANSI_COLOR_RESET " "));
}

/**
 * @brief print the type of file
 * 
 */
void print_type(struct posix_header *header)
{
  switch (header->typeflag)
  {
  case '0':
    write(STDOUT_FILENO, "-", 1);
    break;
  case '5':
    write(STDOUT_FILENO, "d", 1);
    break;
  default:
    write(STDOUT_FILENO, "i", 1);
    break; //type du fichier inconnu
  }
}

/**
 * @brief print the rights of the file
 * 
 */
void print_right(struct posix_header *header)
{
  for (int i = 0; i < 3; i++)
  {
    switch ((header->mode + 4)[i])
    {
    case '0':
      write(STDOUT_FILENO, "---", 3);
      break;
    case '1':
      write(STDOUT_FILENO, "--x", 3);
      break;
    case '2':
      write(STDOUT_FILENO, "-w-", 3);
      break;
    case '3':
      write(STDOUT_FILENO, "-wx", 3);
      break;
    case '4':
      write(STDOUT_FILENO, "r--", 3);
      break;
    case '5':
      write(STDOUT_FILENO, "r-x", 3);
      break;
    case '6':
      write(STDOUT_FILENO, "rw-", 3);
      break;
    case '7':
      write(STDOUT_FILENO, "rwx", 3);
      break;
    }
  }
}

/**
 * @brief print the number of physical links
 * 
 */
void print_nb_link(struct posix_header *header, char *path, int fd)
{
  if (header->typeflag == '5')
  {
    off_t save = lseek(fd, 0, SEEK_CUR);
    lseek(fd, 0, SEEK_SET);

    int nb_link = 2;
    struct posix_header *headerLink = malloc(sizeof(struct posix_header));
    assert(headerLink);
    int n = 0;
    while ((n = read(fd, headerLink, BLOCKSIZE)) > 0)
    {
      if (s_is_in_string(headerLink->name, header->name) && contain_one_char(headerLink->name + strlen(header->name), '/'))
      {
        nb_link++;
      }
      int taille = 0;
      int *ptaille = &taille;
      sscanf(headerLink->size, "%o", ptaille);
      int filesize = ((*ptaille + 512 - 1) / 512);
      lseek(fd, BLOCKSIZE * filesize, SEEK_CUR);
    }

    lseek(fd, save, SEEK_SET);
    free(headerLink);
    char str[12];
    sprintf(str, "%d", nb_link);
    write(STDOUT_FILENO, str, strlen(str));
  }
  else
  {
    write(STDOUT_FILENO, "1", 1);
  }
}

/**
 * @brief print the size of the file
 * 
 */
void print_size(struct posix_header *header, char *path, int fd)
{
  int size;
  sscanf(header->size, "%o", &size);
  char buffer[33];
  if (header->typeflag == '5')
    size = 4096;
  if (size <= 1000000)
  {
    sprintf(buffer, "%d", size);
    print_space(8 - strlen(buffer));
    write(STDOUT_FILENO, buffer, strlen(buffer));
  }
  else if (size <= 1000000000)
  {
    sprintf(buffer, "%d", 1 + size / 1000000);
    print_space(5 - strlen(buffer));
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, " Mo", 3);
  }
  else
  {
    sprintf(buffer, "%d", 1 + size / 1000000000);
    print_space(5 - strlen(buffer));
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, " Go", 3); // je vais m'arrêter là.
  }
}

/**
 * @brief print the time of the last modification on the file
 * 
 */
void print_time(struct posix_header *header)
{
  unsigned long taille;
  sscanf(header->mtime, "%lo", &taille);
  time_t timestamp = taille;
  struct tm *pTime = localtime(&timestamp);
  char buffer[65];
  strftime(buffer, 65, "%b %d %H:%M", pTime);
  print_space(8 - strlen(buffer)); //tant pis si la taille depace 8 carractère
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

/**
 * @brief print the information of the command "ls -l" exept the name
 * 
 */
void print_ls_l(struct posix_header *header, char *path, int fd)
{
  print_type(header);  //affiche le type du fichier
  print_right(header); //affiche les droits
  print_space(1);
  print_nb_link(header, path, fd); //nombre de liens
  print_space(1);
  write(STDOUT_FILENO, header->uname, strlen(header->uname)); //affiche le nom du propriétaire
  print_space(1);
  write(STDOUT_FILENO, header->gname, strlen(header->gname)); //affiche le nom du groupe
  print_space(1);
  print_size(header, path, fd); //affiche la taille du fichier
  print_space(1);
  print_time(header); //affiche l'horodatage
  print_space(1);
}

//affiche ce qu'affiche ls sur un FILE en fonction de la commande
/**
 * @brief print what "ls" or "ls -l" print on one file
 * 
 * @param op the option "-l" or not of the "ls"
 * @param header header of the file
 * @param path path of the file
 */
void print_header_name(char *op, struct posix_header *header, char *path, int fd)
{
  if (isInFolder(path, header->name))
  {
    if (header->typeflag == '5') //Si c'est un rep
    {
      if (op == NULL) //Si pas d'option et rep
      {
        print_name_rep(header, path);
      }
      else if (strcmp(op, "-l") == 0) //Si option et rep
      {
        print_ls_l(header, path, fd);
        print_name_rep(header, path);
        write(STDOUT_FILENO, "\n", strlen("\n"));
      }
      else
        print_name_rep(header, path);
    }
    else
    {
      if (op == NULL)
      {
        print_name_file(header, path);
      }
      else if (strcmp(op, "-l") == 0)
      {
        print_ls_l(header, path, fd);
        print_name_file(header, path);
        write(STDOUT_FILENO, "\n", strlen("\n"));
      }
      else
        print_name_file(header, path);
    }
  }
  else if (strcmpTar(path, header->name) && header->typeflag != '5')
  {
    char *newPath = "\0";
    if (op == NULL)
    {
      print_name_file(header, newPath);
    }
    else if (strcmp(op, "-l") == 0)
    {
      print_ls_l(header, newPath, fd);
      print_name_file(header, newPath);
    }
    else
      print_name_file(header, path);
  }
}

//fonction ls with option -l or not
/**
 * @brief command "ls" and "ls -l" in a tar file
 * 
 * @param op option "-l" or not
 * @param path 
 * @param fd 
 * @return int -1 if it faled and 1 if not
 */
int ls_tar(char *op, char *path, int fd)
{
  struct posix_header *header = malloc(sizeof(struct posix_header));
  assert(header);
  if (fd == -1)
  {
    perror("erreur d'ouverture du fichier");
    return -1;
  }
  if (strlen(path) != 0 && typeFileFd(fd, path) == '9')
  {
    errno = ENOENT;
    perror("tsh: ls");
    return -1;
  }
  int n = 0;
  int error = 0;
  while ((n = read(fd, header, BLOCKSIZE)) > 0)
  {
    if (strlen(header->name) == 0)
      break; //It means we have reached the end of the tar

    if (s_is_in_string(header->name, path))
    {
      error = 1;
      print_header_name(op, header, path, fd);
    }
    int taille = 0;
    int *ptaille = &taille;
    sscanf(header->size, "%o", ptaille);
    int filesize = ((*ptaille + 512 - 1) / 512);
    lseek(fd, BLOCKSIZE * filesize, SEEK_CUR);
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