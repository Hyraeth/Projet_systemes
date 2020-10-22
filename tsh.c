#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "headers/tar_fun.h"
#include "headers/ls_tar.h"
#include "headers/tsh_fun.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int fdTar;
static int tarDepth = -1;
static char **tarDirArray;

#define PWDLEN 1024
#define BUFLEN 512
#define NBARGS 8

typedef struct SimpleCommand_t
{
    int nbargs;
    char **args;
    int nb_options;
    char **options;
} 
SimpleCommand_t;

char *read_line();
SimpleCommand_t *parse_line(char *line);
char **parse_path(char *path);
int exec_cmd(SimpleCommand_t *cmd);
char *get_pwd();

int tsh_cd(SimpleCommand_t *cmd);
int tsh_ls(SimpleCommand_t *cmd);
int tsh_pwd(SimpleCommand_t *cmd);
int tsh_exit(SimpleCommand_t *cmd);


char *builtin_str[] = {
  "cd",
  "ls",
  "pwd",
  "exit"
};

int (*builtin_func[]) (SimpleCommand_t *cmd) = {
  &tsh_cd,
  &tsh_ls,
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
        write(STDOUT_FILENO, "> ", strlen("> "));

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
        for (size_t i = 0; i <= cmd->nb_options; i++)
        {
           free(cmd->options[i]);
        }
        free(cmd->options);
        free(cmd);
        free(pwd);
    } while(run != 0);
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
 * @brief Parse the line into an null-terminated array of args and add it to a SimpleCommand_t struct. 
 * Also add the number of options and an array of option
 * @param line line to parse
 * @return a SimpleCommand_t struct
*/
SimpleCommand_t *parse_line(char *line) {
    int nbargs = NBARGS;

    SimpleCommand_t *cmd = malloc(sizeof(struct SimpleCommand_t));
    char **args = malloc(nbargs*sizeof(char *));
    char **ops = malloc(sizeof(char *));
    char *word = strtok(line, " \n\t");

    int nbword = 0;
    int nbops = 0;
    while(word != NULL) {

        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh");
        }

        if(word[0] == '-' && strlen(word) != 1) {
            ops[nbops] = malloc(strlen(word)+1);
            memcpy(ops[nbops], word, strlen(word)+1);
            nbops++;
            ops = realloc(ops, (nbops+1)*sizeof(int));
        }

        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, " \n\t");
    }
    args[nbword] = NULL;
    ops[nbops] = NULL;
    cmd->args = args;
    cmd->nbargs = nbword;
    cmd->nb_options = nbops;
    cmd->options = ops;

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


int tsh_cd(SimpleCommand_t *cmd) {
    //in case there is an error we can go back
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
            }
            if((tarDirArray = malloc((tarDepth+2)*sizeof(char*))) == NULL) 
                perror("tsh: cd");
            if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1)* sizeof(char))) == NULL) 
                perror("tsh: cd");
            memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
            tarDirArray[tarDepth+1] = NULL;
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
                        tarDirArray = realloc(tarDirArray, (tarDepth+2)*sizeof(char*));
                        tarDirArray[tarDepth+1] = NULL;
                    }
                } //if we go down in the tar
                else if(strcmp(arrayDir[i], ".") != 0) {
                    //if it is a folder connected from where we are in the tar
                    if(isTarFolder(arrayDir[i], tarDirArray) == 1) {
                        tarDepth++;//increment tardepth to add a new folder
                        if((tarDirArray = realloc(tarDirArray, (tarDepth+2) * sizeof(char*))) == NULL) //realloc one size bigger
                            perror("tsh: cd");
                        //malloc the space required for the name of the new folder
                        if((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL) 
                            perror("tsh: cd");
                        //copy the name of the folder into the new space
                        memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
                        tarDirArray[tarDepth+1] = NULL;
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
        memcpy(tarDirArray, tmpTarDir, tmpDepth+1);
        tarDepth = tmpDepth;
        return -1;
    }
    free(pwdtmp);
    free(tmpTarDir);
    return 1;
}

char *array_to_path(char **array) {
    if(array == NULL || array[0] == NULL) return "\0";
    int pathlength = strlen(array[0])+1;
    char *path = malloc(pathlength);
    memcpy(path, array[0], pathlength);
    int i = 1;
    while (array[i] != NULL)
    {
        pathlength += strlen(array[i]);
        path = realloc(path, pathlength+1);
        strcat(path, "/");
        strcat(path, array[i]);
    }
    
    return path;
}

int call_existing_command(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid < 0) {
        perror("lsh");
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else {
        do {
        wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int is_an_option(char *string) {
    return (string[0] == '-') ? true : false;
}

int tsh_ls(SimpleCommand_t *cmd) {
    //if no path is given
    if((cmd->nbargs - cmd->nb_options) == 1) {
        //check whether or not we are in a tar
        if(tarDepth != -1) {//no need to fork since exec is not called
            if(cmd->nb_options > 1 ) {
                write(STDOUT_FILENO, "tsh: ls: Too many options.", strlen("tsh: ls: Too many options."));
                return 1;
            }
            if(cmd->nb_options == 1 && strcmp(cmd->options[0], "-l")==0 || cmd->nb_options == 0) {
                char *path_in_tar = array_to_path(tarDirArray+1);
                ls_tar(cmd->options[0], path_in_tar, fdTar);
                free(path_in_tar);
            }
        } else {
            call_existing_command(cmd->args);
        }
    } //if at least one path is given
    else {
        for (size_t i = 1; i < cmd->nbargs; i++)
        {
            //if the ith argument of the command is not an option
            if(!is_an_option(cmd->args[i])) {
                write(STDOUT_FILENO, cmd->args[i], strlen(cmd->args[i])); //write the name of the path to ls
                //get the absolute path of the path given in form of array of string
                char **abs_path_array = parsePathAbsolute(cmd->args[i], get_pwd()); 
                //split the absolute path in 3 array of string corresponding to the path before the tar, the name of the tar and the path after
                char ***abs_path_split = path_to_tar_file_path_new(abs_path_array);
                //if we are going in a tar
                if(abs_path_split[1] != NULL) {
                    //turn the array of string in the form of a string
                    char *path_to_open = array_to_path(abs_path_split[0]);
                    //add more memory to add the tar to open in the path
                    path_to_open = realloc(path_to_open, strlen(path_to_open) + strlen(abs_path_split[1][0]) + 1);
                    strcat(path_to_open, "/");
                    strcat(path_to_open, abs_path_split[1][0]);
                    //open the tar to ls
                    int fd = open(path_to_open, O_RDWR);
                    //get the path to ls inside the tar
                    char *path_in_tar = array_to_path(abs_path_split[2]);
                    ls_tar(cmd->options[0], path_in_tar, fd);
                } else {
                    char *path_to_ls = array_to_path(abs_path_split[0]);
                    char **args = malloc((cmd->nb_options + 3)*sizeof(char *));
                    memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                    memcpy(args+1, cmd->options, cmd->nb_options);
                    memcpy(args+(cmd->nb_options+1), path_to_ls, 1);
                    call_existing_command(args);
                }
            }
        }
        
    }
    return 1;
}

char *get_pwd() {
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
    return pwd;
}

int tsh_pwd(SimpleCommand_t *cmd) {
    char *pwd = get_pwd();

    write(STDOUT_FILENO, ANSI_COLOR_CYAN, strlen(ANSI_COLOR_CYAN));
    write(STDOUT_FILENO, pwd, strlen(pwd));
    write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
    write(STDOUT_FILENO, "\n", strlen("\n"));
    free(pwd);
    return 1;
}

int tsh_exit(SimpleCommand_t *cmd) {
    return 0;
}