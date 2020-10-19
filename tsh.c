#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFLEN 512
#define NBARGS 8

static int fdTar;
static int tarDepth = -1;
static char **tarDirArray;

typedef struct SimpleCommand_t
{
    int nbargs;
    char **args;
} SimpleCommand_t;

char *read_line();
SimpleCommand_t *parse_line(char *line);
int exec_cmd(SimpleCommand_t *cmd);

int main(int argc, char const *argv[])
{
    int run = 1;
    do {
        char *pwd = getcwd(NULL, 0);
        strcat(pwd, ">");
        write(STDOUT_FILENO, pwd, strlen(pwd));

        char *line;
        SimpleCommand_t *cmd;

        line = read_line();
        cmd = parse_line(line);
        //run = exec_cmd(cmd);

        //free everything
        free(line);
        for (size_t i = 0; i <= cmd->nbargs; i++)
        {
           free(cmd->args[i]);
        }
        free(cmd);
        free(pwd);
    } while(run);
    return 0;
}

/**
 *  @brief Read a line from the standart input
 *  @return The line from the standart input
*/
char *read_line() {
    int bufsize = BUFLEN;
    char *line = malloc(bufsize*sizeof(char));
    if(line == NULL) {
        perror("tsh");
    }
    int n;
    int totalread = 0;
    while ((n = read(STDIN_FILENO, line+totalread, BUFLEN)) > 0) {
        totalread += n;
        //add more space
        if(n == BUFLEN) {
            bufsize += BUFLEN;
            if((line = realloc(line, bufsize)) == NULL)
                perror("tsh");
        }
        //stop
        if(line[totalread-1] == '\n') {
            line[totalread-1] = '\0';
            break;
        }
    }
    //remove excess space
    if((line = realloc(line, totalread)) == NULL)
        perror("tsh");
    //error handling
    if (n < 0)
        perror("tsh: read failed.");
    return line;
}

/**
 *  @brief Parse the line into an null-terminated array of args and add it to a SimpleCommand_t struct
 *  @param line line to parse
 *  @return a SimpleCommand_t struct
*/
SimpleCommand_t *parse_line(char *line) {
    int nbargs = NBARGS;

    SimpleCommand_t *cmd = malloc(sizeof(struct SimpleCommand_t));

    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(line, " \n\t");

    int nbword = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh");
        }

        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, " \n\t");
    }

    if((args = realloc(args, (nbword+1)*sizeof(char *))) == NULL)
        perror("tsh");
    args[nbword] = NULL;
    cmd->args = args;
    cmd->nbargs = nbword;

    return cmd;
}

int tsh_cd(SimpleCommand_t *cmd);
int tsh_exit(SimpleCommand_t *cmd);

char *builtin_str[] = {
  "cd",
  "exit"
};

int (*builtin_func[]) (SimpleCommand_t *cmd) = {
  &tsh_cd,
  &tsh_exit
};

char **parse_path(char *path) {
    int nbargs = NBARGS;

    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(path, "/");

    int nbword = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh");
        }

        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, "/");
    }

    if((args = realloc(args, (nbword+1)*sizeof(char *))) == NULL)
        perror("tsh");
    args[nbword] = NULL;
    return args;
}

int isTar(char *file) {
    if(strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
    return 0;
}

int tsh_cd(SimpleCommand_t *cmd) {
    if(cmd->nbargs > 3) {
        perror("tsh: cd: too many arguments");
    } else if (cmd->nbargs == 2) {
        if(chdir("/") != 0) {
            perror("tsh: cd");
        }
    }
    char **arrayDir = parse_path(cmd->args[1]);
    int i = 0;
    while(arrayDir[i] != NULL) {
        //if the destination a tar
        if(isTar(arrayDir[i])) {
            tarDepth++;
            fdTar = open(arrayDir[i], O_RDWR, S_IRUSR | S_IWUSR);
            if((tarDirArray = malloc(sizeof(char*)))==NULL) 
                perror("tsh");
            if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1)* sizeof(char))) == NULL) 
                perror("tsh");
            memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
        } 
        //if the destination is not a tar
        else {
            //if we are currently in a tar
            if(tarDepth != -1) {
                //if we go up in/out of the tar
                if(strcmp(arrayDir[i], "..") == 0) {
                    free(tarDirArray[tarDepth]);
                    tarDepth--;
                    //if we are now out of the tar
                    if(tarDepth == -1) {
                        close(fdTar);
                        free(tarDirArray);
                    } //if we are not out of the tar
                    else {
                        tarDirArray = realloc(tarDirArray, (tarDepth+1)*sizeof(char*));
                    }
                } //if we go down in the tar
                else if(strcmp(arrayDir[i], ".") != 0) {
                    //if it is a folder connected from where we are in the tar
                    if(isTarFolder(tarDirArray, arrayDir[i]) == 1) {
                        tarDepth++;
                        tarDirArray = realloc(tarDirArray, (tarDepth+1)*sizeof(char*));
                        if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1)* sizeof(char))) == NULL) 
                            perror("tsh");
                        memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
                    }
                    //if it is not a folder connected from where we are
                    else {
                        perror("tsh: cd: No such directory");
                    }
                }
            } //if we are not in a tar
            else {
                if(chdir(arrayDir[i]) != 0) {
                    perror("tsh: cd");
                }
            }
        }
        i++;
    }
    return 1;
}