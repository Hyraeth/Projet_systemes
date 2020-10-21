#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define PWDLEN 1024
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
int tsh_cd(SimpleCommand_t *cmd);
int tsh_pwd(SimpleCommand_t *cmd);
int tsh_exit(SimpleCommand_t *cmd);
char **parse_path(char *path);

char *builtin_str[] = {
  "cd",
  "pwd",
  "exit"
};

int (*builtin_func[]) (SimpleCommand_t *cmd) = {
  &tsh_cd,
  &tsh_pwd,
  &tsh_exit
};

int main(int argc, char const *argv[])
{
    int run = 1;
    do {
        int pwdlen = PWDLEN;
        char *pwd = malloc(pwdlen); 
        while(getcwd(pwd, pwdlen) == NULL) {
            pwdlen *= 2;
            free(pwd);
            pwd = malloc(pwdlen);
        }
        if(tarDepth > -1) {
            for (size_t i = 0; i <= tarDepth; i++)
            {   
                strcat(pwd,"/");
                strcat(pwd, tarDirArray[i]);
            }
        }
        
        
        write(STDOUT_FILENO, ANSI_COLOR_BLUE, strlen(ANSI_COLOR_BLUE));
        write(STDOUT_FILENO, pwd, strlen(pwd));
        write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
        write(STDOUT_FILENO, ">", strlen(">"));

        char *line;
        SimpleCommand_t *cmd;

        line = read_line();
        cmd = parse_line(line);
        run = exec_cmd(cmd);

        //free everything
        free(line);
        for (size_t i = 0; i <= cmd->nbargs; i++)
        {
           free(cmd->args[i]);
        }
        free(cmd->args);
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
    args[nbword] = NULL;
    cmd->args = args;
    cmd->nbargs = nbword;

    return cmd;
}

int exec_cmd(SimpleCommand_t *cmd) {
    if(cmd->args[0] == NULL) {
        return 1;
    }
    for (size_t i = 0; i < 2; i++)
    {
        if (strcmp(cmd->args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(cmd);
        }
    }
    return 0;
}

char **parse_path(char *path) {
    int nbargs = NBARGS;

    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(path, "/");

    int nbword = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh realloc parse path");
        }

        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh malloc parse path");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, "/");
    }
    args[nbword] = NULL;
    return args;
}

int isTar(char *file) {
    if(strcmp(file+(strlen(file)-4), ".tar") == 0) return 1;
    return 0;
}

int isTarFolder(char *folder, char **path) {
    return 1;
}

int tsh_cd(SimpleCommand_t *cmd) {
    int pwdlen = PWDLEN;
    char *pwdtmp = malloc(pwdlen); 
    while(getcwd(pwdtmp, pwdlen) == NULL) {
        pwdlen *= 2;
        free(pwdtmp);
        pwdtmp = malloc(pwdlen);
    }
    int tmpDepth = tarDepth;
    char **tmpTarDir = malloc((tarDepth+1)*sizeof(char *));
    memcpy(tmpTarDir, tarDirArray, tarDepth+1);
    int failure = 0;
    //weirds arguments handling
    if(cmd->nbargs > 2) {
        write(STDOUT_FILENO, "tsh: cd: too many arguments\n", strlen("tsh: cd: too many arguments\n"));
        return 1;
    } //if we want to go to home or ~
    if(cmd->nbargs == 1) {
        char *home = getenv ("HOME");
        if(chdir(home) != 0) {
            perror("tsh: cd");
        }
        return 1;
    }
    int i = 0;
    //if the path start by '/'
    if(cmd->args[1][0] == '/') {
        if(chdir("/") != 0) {
            perror("tsh: cd");
        }
    } 
    //if the parth start with a ~
    if(cmd->args[1][0] == '~') {
        char *home = getenv ("HOME");
        if(chdir(home) != 0) {
            perror("tsh: cd");
        }
        i++;
    }
    char *path = malloc(strlen(cmd->args[1])+1);
    memcpy(path, cmd->args[1], strlen(cmd->args[1])+1);
    char **arrayDir = parse_path(path); //parsing th path into array of folder/file name
    while(arrayDir[i] != NULL) {
        //if the destination a tar we add the file name into tarDirArray
        if(isTar(arrayDir[i])) {
            tarDepth++;
            if((fdTar = open(arrayDir[i], O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
                free(path);
                tarDepth--;
                perror("tsh: cd");
                return 1;
            }
            if((tarDirArray = malloc((tarDepth+1)*sizeof(char*))) == NULL) 
                perror("tsh: cd");
            if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1)* sizeof(char))) == NULL) 
                perror("tsh: cd");
            memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
        } 
        //if the destination is not a tar
        else {
            //if we are currently in a tar
            if(tarDepth != -1) {
                //if we go up in/out of the tar
                if(strcmp(arrayDir[i], "..") == 0) {
                    free(tarDirArray[tarDepth]); //we free the latest folder added to tarDirArray
                    tarDepth--; //decrement tardepth since we took out the latest folder
                    //if we are now out of the tar
                    if(tarDepth == -1) {
                        fdTar = close(fdTar); //we can now close the file descriptor
                        free(tarDirArray); // and free the tarDirArray entirely (we will malloc again if we go back into a tar)
                    } //if we are not out of the tar
                    else {
                        //we realloc tarDirArray into 1 size smaller
                        tarDirArray = realloc(tarDirArray, (tarDepth+1)*sizeof(char*));
                    }
                } //if we go down in the tar
                else if(strcmp(arrayDir[i], ".") != 0) {
                    //if it is a folder connected from where we are in the tar
                    if(isTarFolder(arrayDir[i], tarDirArray) == 1) {
                        tarDepth++;//increment tardepth to add a new folder
                        if((tarDirArray = realloc(tarDirArray, (tarDepth+1) * sizeof(char*))) == NULL) //realloc one size bigger
                            perror("tsh: cd");
                        //malloc the space required for the name of the new folder
                        if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL) 
                            perror("tsh: cd");
                        //copy the name of the folder into the new space
                        memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
                    }
                    //if it is not a folder connected from where we are
                    else {
                        failure = 1;
                        write(STDOUT_FILENO, "tsh: cd: No such directory\n", strlen("tsh: cd: No such directory\n"));
                        break;
                    }
                }
            } //if we are not in a tar
            else {
                if(chdir(arrayDir[i]) != 0) {
                    failure = 1;
                    write(STDOUT_FILENO, "tsh: cd: No such directory\n", strlen("tsh: cd: No such directory\n"));
                    break;
                }
            }
        }
        i++;
    }
    free(arrayDir);
    free(path);
    if(failure) {
        chdir(pwdtmp);
        free(pwdtmp);
        memcpy(tarDirArray, tmpTarDir, tmpDepth+1);
        tarDepth = tmpDepth;
        free(tmpTarDir);
    }

    return 1;
}

int tsh_pwd(SimpleCommand_t *cmd) {
    int pwdlen = PWDLEN;
    char *pwd = malloc(pwdlen); 
    while(getcwd(pwd, pwdlen) == NULL) {
        pwdlen *= 2;
        free(pwd);
        pwd = malloc(pwdlen);
    }
    if(tarDepth > -1) {
        for (size_t i = 0; i <= tarDepth; i++)
        {   
            strcat(pwd,"/");
            strcat(pwd, tarDirArray[i]);
        }
    }

    write(STDOUT_FILENO, ANSI_COLOR_CYAN, strlen(ANSI_COLOR_CYAN));
    write(STDOUT_FILENO, pwd, strlen(pwd));
    write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
    write(STDOUT_FILENO, "\n", strlen("\n"));
    return 1;
}

int tsh_exit(SimpleCommand_t *cmd) {
    return 0;
}