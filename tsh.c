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

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

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
} SimpleCommand_t;

typedef struct ComplexCommand_t
{
    int nbcmd;
    SimpleCommand_t **simpCmds;
    char *input;
    char *output;
    char *err;
    int appendOut;
    int appendErr;
} ComplexCommand_t;

void free_simplecmd(SimpleCommand_t *cmd);
void free_complexcmd(ComplexCommand_t *cmd);
char *read_line();
ComplexCommand_t *parse_line(char *line);
SimpleCommand_t *parse_simpCmd(char *line);
char **parse_path(char *path);
int exec_cmd(SimpleCommand_t *cmd);
int exec_complexcmd(ComplexCommand_t *cmd);
int call_existing_command(char **args);
char *get_pwd();

int tsh_cd(SimpleCommand_t *cmd);
int tsh_ls(SimpleCommand_t *cmd);
int tsh_pwd(SimpleCommand_t *cmd);
int tsh_exit(SimpleCommand_t *cmd);

char *builtin_str[] = {
    "cd",
    "ls",
    "pwd",
    "exit"};

/**
 * @brief List of builtin functions currently implemented.
 * 
 */
int (*builtin_func[])(SimpleCommand_t *cmd) = {
    &tsh_cd,
    &tsh_ls,
    &tsh_pwd,
    &tsh_exit};

int main(int argc, char const *argv[])
{
    int run = 1;
    do
    {
        int pwdlen = PWDLEN;
        char *pwd;
        if ((pwd = malloc(pwdlen)) == NULL)
            perror("tsh");
        while (getcwd(pwd, pwdlen) == NULL)
        {
            pwdlen *= 2;
            free(pwd);
            if ((pwd = malloc(pwdlen)) == NULL)
                perror("tsh");
        }
        if (tarDepth > -1)
        {
            for (size_t i = 0; i <= tarDepth; i++)
            {
                strcat(pwd, "/");
                strcat(pwd, tarDirArray[i]);
            }
        }

        write(STDOUT_FILENO, ANSI_COLOR_BLUE, strlen(ANSI_COLOR_BLUE));
        write(STDOUT_FILENO, pwd, strlen(pwd));
        write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
        write(STDOUT_FILENO, "> ", strlen("> "));

        char *line;
        ComplexCommand_t *cmd;

        line = read_line();
        cmd = parse_line(line);
        for (size_t i = 0; i < cmd->nbcmd; i++)
        {
            write(1, &cmd->nbcmd, sizeof(cmd->nbcmd));
            write(1, cmd->simpCmds[i]->args[0], strlen(cmd->simpCmds[i]->args[0]));
            write(1, "\n", 1);
        }
        run = exec_complexcmd(cmd);

        //free everything
        free(line);
        free_complexcmd(cmd);
        free(pwd);
    } while (run != 0);
    return 0;
}

void free_simplecmd(SimpleCommand_t *cmd)
{
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
}

void free_complexcmd(ComplexCommand_t *cmd)
{
    //free(cmd->input);
    //free(cmd->output);
    //free(cmd->err);
    for (size_t i = 0; i < cmd->nbcmd; i++)
    {
        free_simplecmd(cmd->simpCmds[i]);
    }
    free(cmd);
}

/**
 *  @brief Read a line from the standart input
 *  @return The line from the standart input
*/
char *read_line()
{
    int bufsize = BUFLEN;
    char *line;
    if ((line = malloc(bufsize * sizeof(char))) == NULL)
        perror("tsh");
    if (line == NULL)
    {
        perror("tsh");
    }
    int n;
    int totalread = 0;
    while ((n = read(STDIN_FILENO, line + totalread, BUFLEN)) > 0)
    {
        totalread += n;
        //add more space
        if (n == BUFLEN)
        {
            bufsize += BUFLEN;
            if ((line = realloc(line, bufsize)) == NULL)
                perror("tsh");
        }
        //stop
        if (line[totalread - 1] == '\n')
        {
            line[totalread - 1] = '\0';
            break;
        }
    }
    //error handling
    if (n < 0)
        perror("tsh: read failed.");
    return line;
}

ComplexCommand_t *parse_line(char *line)
{
    int simpCmdArraySize = NBARGS;
    ComplexCommand_t *cmd = malloc(sizeof(ComplexCommand_t));
    SimpleCommand_t **simpCmds = malloc(simpCmdArraySize * sizeof(SimpleCommand_t *));
    if (cmd == NULL || simpCmds == NULL)
        perror("tsh");

    //TODO : find redirections
    cmd->input = "";
    cmd->output = "";
    cmd->err = "";
    write(1, line, strlen(line));

    //parsing simple cmds
    char *cmdString = strtok(line, "|");
    int nbcmd = 0;
    while (cmdString != NULL)
    {
        if (nbcmd == simpCmdArraySize)
        {
            simpCmdArraySize += NBARGS;
            if ((simpCmds = realloc(simpCmds, simpCmdArraySize * sizeof(SimpleCommand_t *))) == NULL)
                perror("tsh");
        }
        char *tmpline = malloc(strlen(cmdString) + 1);
        memcpy(tmpline, cmdString, strlen(cmdString) + 1);
        simpCmds[nbcmd] = parse_simpCmd(tmpline);
        nbcmd++;
        free(tmpline);
        cmdString = strtok(NULL, "|");
    }
    simpCmds[nbcmd] = NULL;
    cmd->simpCmds = simpCmds;
    cmd->nbcmd = nbcmd;
}

/**
 * @brief Parse the line into a null-terminated array of args and add it to a SimpleCommand_t struct. 
 * Also add the number of options and an array of option
 * @param line Line to parse
 * @return a SimpleCommand_t struct
*/
SimpleCommand_t *parse_simpCmd(char *line)
{
    int nbargs = NBARGS;

    SimpleCommand_t *cmd = malloc(sizeof(SimpleCommand_t));
    char **args = malloc(nbargs * sizeof(char *));
    char **ops = malloc(sizeof(char *));
    if (args == NULL)
        perror("tsh");
    if (cmd == NULL)
        perror("tsh");
    if (ops == NULL)
        perror("tsh");
    char *word = strtok(line, " \n\t");

    int nbword = 0;
    int nbops = 0;
    while (word != NULL)
    {

        if (nbword == nbargs)
        {
            nbargs += NBARGS;
            if ((args = realloc(args, nbargs * sizeof(char *))) == NULL)
                perror("tsh");
        }

        if (word[0] == '-' && strlen(word) != 1)
        {
            if ((ops[nbops] = malloc(strlen(word) + 1)) == NULL)
                perror("tsh");
            memcpy(ops[nbops], word, strlen(word) + 1);
            nbops++;
            if ((ops = realloc(ops, (nbops + 1) * sizeof(int))) == NULL)
                perror("tsh");
        }

        if ((args[nbword] = malloc((strlen(word) + 1) * sizeof(char))) == NULL)
            perror("tsh");
        memcpy(args[nbword], word, strlen(word) + 1);
        if (word[strlen(word) - 1] == '\\')
        {
            int newsize = strlen(word) + 1;
            word = strtok(NULL, " \n\t");
            newsize += strlen(word) + 1;
            if ((args[nbword] = realloc(args[nbword], newsize)) == NULL)
                perror("tsh");
            strcat(args[nbword], " ");
            strcat(args[nbword], word);
        }
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

/**
 * @brief Execute an existing command in the PATH if successful.
 * 
 * @param args Null-terminated array containing the command split using " "(space) as a separator.
 * @return 1 no matter what
 */
int call_existing_command(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        perror("tsh");
    }
    else if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("tsh");
        }
        exit(EXIT_FAILURE);
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
 * @brief Execute a command. If the command is builtin then call the function to execute it. If it isn't it will try to find the command 
 * in the PATH environnemental variable
 * 
 * @param cmd Command to execute
 * @return 1 if successful, 0 if exit was called, -1 in case of failure. 
 */
int exec_cmd(SimpleCommand_t *cmd)
{
    if (cmd->args[0] == NULL)
    {
        return 1;
    }
    for (size_t i = 0; i < 4; i++)
    {
        if (strcmp(cmd->args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(cmd);
        }
    }
    if (tarDepth == -1)
    {
        return call_existing_command(cmd->args);
    }
    else
    {
        write(STDERR_FILENO, ANSI_COLOR_RED, strlen(ANSI_COLOR_RED));
        write(STDERR_FILENO, "tsh: can't call ", strlen("tsh: can't call "));
        write(STDERR_FILENO, cmd->args[0], strlen(cmd->args[0]));
        write(STDERR_FILENO, " in a tar. First use cd to get out.\n", strlen(" in a tar. First use cd to get out.\n"));
        write(STDERR_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
        return 1;
    }
}

int exec_complexcmd(ComplexCommand_t *cmd)
{
    int fdin, fdout, fderr, status, retval;
    //save current file descriptor
    int tmpin = dup(STDIN_FILENO);
    int tmpout = dup(STDOUT_FILENO);
    int tmperr = dup(STDERR_FILENO);

    if (strcmp(cmd->input, "") == 0)
    {
        fdin = dup(tmpin);
    }
    else
    {
        //todo
        fdin = dup(tmpin);
    }
    if (strcmp(cmd->err, "") == 0)
    {
        fderr = dup(tmperr);
    }
    else
    {
        //todo
        fderr = dup(tmperr);
    }
    dup2(fderr, STDERR_FILENO);
    close(fderr);
    //if there's only one simple command
    if (cmd->nbcmd == 1)
    {
        //Change err and std output
        if (strcmp(cmd->output, "") == 0)
        {
            fdout = dup(tmpout);
        }
        else
        {
            //todo
            fdout = dup(tmpout);
        }

        dup2(fdin, STDIN_FILENO);
        close(fdin);
        dup2(fdout, STDOUT_FILENO);
        close(fdout);
        retval = exec_cmd(cmd->simpCmds[0]);
        dup2(tmpin, STDIN_FILENO);
        close(tmpin);
        dup2(tmpout, STDOUT_FILENO);
        close(tmpout);
        dup2(tmperr, STDERR_FILENO);
        close(tmperr);
        return retval;
    }
    pid_t cpid, wpid;
    for (size_t i = 0; i < cmd->nbcmd; i++)
    {
        dup2(fdin, STDIN_FILENO);
        close(fdin);
        if (i == cmd->nbcmd - 1)
        {
            if (strcmp(cmd->output, "") == 0)
            {
                fdout = dup(tmpout);
            }
            else
            {
                //todo
                fdout = dup(tmpout);
            }
        }
        else
        {
            int fdpipe[2];
            if (pipe(fdpipe) == -1)
                perror("tsh");
            fdin = fdpipe[0];
            fdout = fdpipe[1];
        }
        dup2(fdout, STDOUT_FILENO);
        close(fdout);

        cpid = fork();
        if (cpid == -1)
            perror("tsh");
        if (cpid == 0)
        {
            if (exec_cmd(cmd->simpCmds[i]) == -1)
                retval = -1;
        }
    }
    dup2(tmpin, STDIN_FILENO);
    close(tmpin);
    dup2(tmpout, STDOUT_FILENO);
    close(tmpout);
    dup2(tmperr, STDERR_FILENO);
    close(tmperr);

    do
    {
        wpid = waitpid(cpid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    return 1;
}

/**
 * @brief Parse the path into a null terminaled array of strings. The path is split using "/" as a separator
 * 
 * @param path Path to parse
 * @return A null terminaled array of strings.
 */
char **parse_path(char *path)
{
    int nbargs = NBARGS;

    char **args;
    if ((args = malloc(nbargs * sizeof(char *))) == NULL)
        perror("tsh");
    char *word = strtok(path, "/");

    int nbword = 0;
    while (word != NULL)
    {

        if (nbword == nbargs)
        {
            nbargs += NBARGS;
            if ((args = realloc(args, nbargs * sizeof(char *))) == NULL)
                perror("tsh realloc parse path");
        }

        if ((args[nbword] = malloc((strlen(word) + 1) * sizeof(char))) == NULL)
            perror("tsh malloc parse path");
        memcpy(args[nbword], word, strlen(word) + 1);

        nbword++;
        word = strtok(NULL, "/");
    }
    args[nbword] = NULL;
    return args;
}

int cd_tar(char *path)
{
    //in case there is an error we can go back
    int pwdlen = PWDLEN;
    char *pwdtmp;
    if ((pwdtmp = malloc(pwdlen)) == NULL)
        perror("tsh");
    while (getcwd(pwdtmp, pwdlen) == NULL)
    {
        pwdlen *= 2;
        free(pwdtmp);
        if ((pwdtmp = malloc(pwdlen)) == NULL)
            perror("tsh");
    }
    int tmpDepth = tarDepth;
    char **tmpTarDir;
    if ((tmpTarDir = malloc((tarDepth + 1) * sizeof(char *))) == NULL)
        perror("tsh");
    memcpy(tmpTarDir, tarDirArray, tarDepth + 1);
    int failure = 0;

    int i = 0;
    //if the path start by '/'
    if (path[0] == '/')
    {
        if (chdir("/") != 0)
        {
            perror("tsh: cd");
        }
    }
    //if the parth start with a ~
    if (path[0] == '~')
    {
        char *home = getenv("HOME");
        if (chdir(home) != 0)
        {
            perror("tsh: cd");
        }
        i++;
    }
    char *chemin;
    if ((chemin = malloc(strlen(path) + 1)) == NULL)
        perror("tsh");
    memcpy(chemin, path, strlen(path) + 1);
    char **arrayDir = parse_path(chemin); //parsing th path into array of folder/file name
    while (arrayDir[i] != NULL)
    {
        //if the destination a tar we add the file name into tarDirArray
        remove_escape_char(arrayDir[i]);
        if (isTar(arrayDir[i]))
        {
            tarDepth++;
            if ((fdTar = open(arrayDir[i], O_RDWR, S_IRUSR | S_IWUSR)) == -1)
            {
                free(chemin);
                tarDepth--;
                perror("tsh: cd");
            }
            if ((tarDirArray = malloc((tarDepth + 2) * sizeof(char *))) == NULL)
                perror("tsh: cd");
            if ((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL)
                perror("tsh: cd");
            memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
            tarDirArray[tarDepth + 1] = NULL;
        }
        //if the destination is not a tar
        else
        {
            //if we are currently in a tar
            if (tarDepth != -1)
            {
                //if we go up in/out of the tar
                if (strcmp(arrayDir[i], "..") == 0)
                {
                    free(tarDirArray[tarDepth]); //we free the latest folder added to tarDirArray
                    tarDepth--;                  //decrement tardepth since we took out the latest folder
                    //if we are now out of the tar
                    if (tarDepth == -1)
                    {
                        fdTar = close(fdTar); //we can now close the file descriptor
                        free(tarDirArray);    // and free the tarDirArray entirely (we will malloc again if we go back into a tar)
                    }                         //if we are not out of the tar
                    else
                    {
                        //we realloc tarDirArray into 1 size smaller
                        if ((tarDirArray = realloc(tarDirArray, (tarDepth + 2) * sizeof(char *))) == NULL)
                            perror("tsh: cd");
                        tarDirArray[tarDepth + 1] = NULL;
                    }
                } //if we go down in the tar
                else if (strcmp(arrayDir[i], ".") != 0)
                {
                    //if it is a folder connected from where we are in the tar
                    if (isTarFolder(arrayDir[i], tarDirArray) == 1)
                    {
                        tarDepth++;                                                                        //increment tardepth to add a new folder
                        if ((tarDirArray = realloc(tarDirArray, (tarDepth + 2) * sizeof(char *))) == NULL) //realloc one size bigger
                            perror("tsh: cd");
                        //malloc the space required for the name of the new folder
                        if ((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL)
                            perror("tsh: cd");
                        //copy the name of the folder into the new space
                        memcpy(tarDirArray[tarDepth], arrayDir[i], strlen(arrayDir[i]) + 1);
                        tarDirArray[tarDepth + 1] = NULL;
                    }
                    //if it is not a folder connected from where we are
                    else
                    {
                        failure = 1;
                        write(STDERR_FILENO, "tsh: cd: No such directory\n", strlen("tsh: cd: No such directory\n"));
                        break;
                    }
                }
            } //if we are not in a tar
            else
            {
                if (chdir(arrayDir[i]) != 0)
                {
                    failure = 1;
                    write(STDERR_FILENO, "tsh: cd: No such directory\n", strlen("tsh: cd: No such directory\n"));
                    break;
                }
            }
        }
        i++;
    }
    free(arrayDir);
    free(chemin);
    if (failure)
    {
        chdir(pwdtmp);
        memcpy(tarDirArray, tmpTarDir, tmpDepth + 1);
        tarDepth = tmpDepth;
        return -1;
    }
    free(pwdtmp);
    free(tmpTarDir);
    return 1;
}

/**
 * @brief Execute the command "cd". For more information see ARCHITECTURE.md
 * 
 * @param cmd Command to execute. Contains the path and the number of arguments of the command
 * @return 1 on success, -1 on failure 
 */
int tsh_cd(SimpleCommand_t *cmd)
{

    //weirds arguments handling
    if (cmd->nbargs > 2)
    {
        write(STDERR_FILENO, "tsh: cd: too many arguments\n", strlen("tsh: cd: too many arguments\n"));
        return 1;
    } //if we want to go to home or ~
    if (cmd->nbargs == 1)
    {
        char *home = getenv("HOME");
        if (chdir(home) != 0)
        {
            perror("tsh: cd");
        }
        return 1;
    }
    return cd_tar(cmd->args[1]);
}

/**
 * @brief Execute the command "ls".  
 * 
 * @param cmd Command to execute. Contains the path and options
 * @return 1 on success 
 */
int tsh_ls(SimpleCommand_t *cmd)
{
    //if no path is given
    if ((cmd->nbargs - cmd->nb_options) == 1)
    {
        //check whether or not we are in a tar
        if (tarDepth != -1)
        { //no need to fork since exec is not called
            if (cmd->nb_options > 1)
            {
                write(STDERR_FILENO, "tsh: ls: Too many options.", strlen("tsh: ls: Too many options."));
                return 1;
            }
            if (cmd->nb_options == 1 && strcmp(cmd->options[0], "-l") == 0 || cmd->nb_options == 0)
            {
                char *path_in_tar = array_to_path(tarDirArray + 1, 0);
                ls_tar(cmd->options[0], path_in_tar, fdTar);
                free(path_in_tar);
            }
        }
        else
        {
            call_existing_command(cmd->args);
        }
    } //if at least one path is given
    else
    {
        for (size_t i = 1; i < cmd->nbargs; i++)
        {
            //if the ith argument of the command is not an option
            if (!is_an_option(cmd->args[i]))
            {
                write(STDOUT_FILENO, "\n", strlen("\n"));
                write(STDOUT_FILENO, ANSI_COLOR_CYAN, strlen(ANSI_COLOR_CYAN));
                write(STDOUT_FILENO, cmd->args[i], strlen(cmd->args[i])); //write the name of the path to ls
                write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
                write(STDOUT_FILENO, ":\n", strlen(":\n"));
                //get the absolute path of the path given in form of array of string
                char **abs_path_array = parsePathAbsolute(cmd->args[i], get_pwd());
                if (abs_path_array == NULL)
                    return -1;
                remove_escape_char_array(abs_path_array);

                //split the absolute path in 3 array of string corresponding to the path before the tar, the name of the tar and the path after
                char ***abs_path_split = path_to_tar_file_path_new(abs_path_array);
                //if we are going in a tar
                if (abs_path_split[1] != NULL)
                {
                    //turn the array of string in the form of a string
                    char *path_to_open = array_to_path(abs_path_split[0], 1);
                    //add more memory to add the tar to open in the path
                    if ((path_to_open = realloc(path_to_open, strlen(path_to_open) + strlen(abs_path_split[1][0]) + 1)) == NULL)
                        perror("tsh: ls");
                    strcat(path_to_open, "/");
                    strcat(path_to_open, abs_path_split[1][0]);
                    //open the tar to ls
                    int fd;
                    if ((fd = open(path_to_open, O_RDWR, S_IRUSR | S_IWUSR)) == -1)
                        perror("tsh: ls");
                    //get the path to ls inside the tar
                    char *path_in_tar = array_to_path(abs_path_split[2], 0);
                    ls_tar(cmd->options[0], path_in_tar, fd);
                    free(path_to_open);
                    free(path_in_tar);
                    close(fd);
                }
                else
                {
                    //get the path to ls
                    char *path_to_ls = array_to_path(abs_path_array, 1);
                    if (strcmp(path_to_ls, "") == 0)
                        path_to_ls[0] = '/';
                    //allocate memory for "ls", the options, and the path
                    char **args = malloc((cmd->nb_options + 3) * sizeof(char *));
                    args[0] = malloc(strlen(cmd->args[0]) + 1);
                    memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                    for (size_t i = 0; i < cmd->nb_options; i++)
                    {
                        args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                        memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                    }
                    args[cmd->nb_options + 1] = malloc(strlen(path_to_ls) + 1);
                    memcpy(args[cmd->nb_options + 1], path_to_ls, strlen(path_to_ls) + 1);
                    args[cmd->nb_options + 2] = NULL;
                    //write(1, path_to_ls, strlen(path_to_ls));
                    free(path_to_ls);

                    call_existing_command(args);
                }
                int i = 0;
                while (abs_path_array[i] != NULL)
                {
                    free(abs_path_array[i]);
                    i++;
                }

                free(abs_path_array);
                for (size_t j = 0; j <= 2; j++)
                {

                    if (abs_path_split[j] != NULL)
                    {
                        int k = 0;
                        while (abs_path_split[j][k] != NULL)
                        {
                            //free(abs_path_split[j][k]);
                            k++;
                        }
                        free(abs_path_split[j]);
                    }
                }
                free(abs_path_split);
            }
        }
    }
    return 1;
}

/**
 * @brief Get the pwd object
 * 
 * @return the absolute path to the current directory in form of a string.
 */
char *get_pwd()
{
    int pwdlen = PWDLEN;
    char *pwd = malloc(pwdlen);
    while (getcwd(pwd, pwdlen) == NULL)
    {
        pwdlen *= 2;
        free(pwd);
        pwd = malloc(pwdlen);
    }
    if (tarDepth > -1)
    {
        for (size_t i = 0; i <= tarDepth; i++)
        {
            strcat(pwd, "/");
            strcat(pwd, tarDirArray[i]);
        }
    }
    return pwd;
}

/**
 * @brief Write the pwd into the standard output.
 * 
 * @param cmd Irrelevant. like myself.
 * @return 1 whatever happens
 */
int tsh_pwd(SimpleCommand_t *cmd)
{
    char *pwd = get_pwd();

    write(STDOUT_FILENO, ANSI_COLOR_CYAN, strlen(ANSI_COLOR_CYAN));
    write(STDOUT_FILENO, pwd, strlen(pwd));
    write(STDOUT_FILENO, ANSI_COLOR_RESET, strlen(ANSI_COLOR_RESET));
    write(STDOUT_FILENO, "\n", strlen("\n"));
    free(pwd);
    return 1;
}

/**
 * @brief Exit the terminal by ending the main loop.
 * 
 * @param cmd Decoration
 * @return 0 
 */
int tsh_exit(SimpleCommand_t *cmd)
{
    return 0;
}