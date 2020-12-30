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
#include "headers/cat_tar.h"
#include "headers/tsh_fun.h"
#include "headers/cp_tar.h"
#include "headers/rm_tar.h"
#include "headers/mkdir_tar.h"
#include "headers/rmdir_tar.h"
#include "headers/mv_tar.h"

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
/**
 * @brief Struct for a simple command.
 * Contains the number of argument in the command (ie the command name, the options and the command args).
 * As well as a null-terminated array of args.
 * Also contains the number of option as well as a null terminated array of option.
 * args also contains the options.
 */
typedef struct SimpleCommand_t
{
    int nbargs;
    char **args;
    int nb_options;
    char **options;
} SimpleCommand_t;

/**
 * @brief Struct for a complex command
 * Contains the number of simple command, a null terminated array of simple commands, three strings for the inpuit, output, err path for redirection,
 * and two ints for the output and err if we want to append.
 */
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
pathStruct *makeStructFromPath(char *path);
int has_correct_option(char **optionlist, const char *option);

int tsh_cd(SimpleCommand_t *cmd);
int tsh_cat(SimpleCommand_t *cmd);
int tsh_ls(SimpleCommand_t *cmd);
int tsh_pwd(SimpleCommand_t *cmd);
int tsh_exit(SimpleCommand_t *cmd);
int tsh_cp(SimpleCommand_t *cmd);
int tsh_rm(SimpleCommand_t *cmd);
int tsh_rmdir(SimpleCommand_t *cmd);
int tsh_mkdir(SimpleCommand_t *cmd);
int tsh_mv(SimpleCommand_t *cmd);

char *builtin_str[] = {
    "cd",
    "cat",
    "ls",
    "cp",
    "rm",
    "pwd",
    "mkdir",
    "rmdir",
    "mv",
    "exit"};

/**
 * @brief List of builtin functions currently implemented.
 * 
 */
int (*builtin_func[])(SimpleCommand_t *cmd) = {
    &tsh_cd,
    &tsh_cat,
    &tsh_ls,
    &tsh_cp,
    &tsh_rm,
    &tsh_pwd,
    &tsh_mkdir,
    &tsh_rmdir,
    &tsh_mv,
    &tsh_exit};

int main(int argc, char const *argv[])
{
    int run = 1;
    do
    {
        int pwdlen = PWDLEN;
        char *pwd;
        if ((pwd = malloc(pwdlen)) == NULL)
            perror("tsh: pwd malloc");
        while (getcwd(pwd, pwdlen) == NULL)
        {
            pwdlen *= 2;
            free(pwd);
            if ((pwd = malloc(pwdlen)) == NULL)
                perror("tsh: pwd malloc");
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
        /*
        write(1, "line read : \n", strlen("line read : \n"));
        write(1, line, strlen(line));
        write(1, "\nend line read \n\n", strlen("\nend line read \n\n"));
        */
        cmd = parse_line(line);
        /*
        write(1, "cmd args : \n", strlen("cmd args : \n"));
        for (size_t i = 0; i < cmd->nbcmd; i++)
        {
            for (size_t j = 0; j < cmd->simpCmds[i]->nbargs; j++)
            {
                write(1, cmd->simpCmds[i]->args[j], strlen(cmd->simpCmds[i]->args[j]));
                write(1, " ", 1);
            }
            write(1, "\n", 1);
        }
        write(1, "end cmd args\n\n", strlen("end cmd args\n\n"));
        */
        run = exec_complexcmd(cmd);

        //free everything
        free(line);
        free_complexcmd(cmd);
        free(pwd);
    } while (run != 0);
    return 0;
}

/**
 * @brief Free a SimpleCommand_t
 * 
 * @param cmd SimpleCommand_t to be freed
 */
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
    cmd->nb_options = 0;
    cmd->nbargs = 0;
    free(cmd->options);
    free(cmd);
}

/**
 * @brief Free a ComplexCommand_t
 * 
 * @param cmd ComplexCommand_t to be freed
 */
void free_complexcmd(ComplexCommand_t *cmd)
{

    free(cmd->input);
    free(cmd->output);
    free(cmd->err);
    cmd->appendErr = 0;
    cmd->appendOut = 0;
    for (size_t i = 0; i < cmd->nbcmd; i++)
    {
        free_simplecmd(cmd->simpCmds[i]);
    }
    free(cmd->simpCmds);
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
        perror("tsh: read_line malloc");
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
                perror("tsh: read_line realloc");
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
        perror("tsh: read_line failed.");
    return line;
}

/**
 * @brief Find input path from a string and add it to the ComplexCommand_t if found
 * If the input char '<' is found, the next word is added to the command and removed from the string.
 * If nothing is found, an empty string will be added to cmd
 * @param line String to analyse.
 * @param ccmd ComplexCommand_t  where the string will be stored.
 */
void findInput(char *line, ComplexCommand_t *ccmd)
{
    int s = 0;
    int e = 0;
    for (size_t i = 0; i < strlen(line); i++)
    {
        if (line[i] == '<')
        {
            //find start of input path
            for (size_t j = i + 1; j < strlen(line); j++)
            {
                if (line[j] != ' ')
                {
                    s = j;
                    break;
                }
            }
            //if no start then remove input char and break;
            if (s == 0)
            {
                line[i] = ' ';
                break;
            }
            //find end of input path
            for (size_t j = s; j < strlen(line) + 1; j++)
            {
                if (line[j] == ' ' || line[j] == '\0')
                {
                    e = j;
                    break;
                }
            }
            line[i] = ' ';
            break;
        }
    }
    if (s == 0)
    {
        ccmd->input = malloc(1);
        ccmd->input[0] = '\0';
    }
    else
    {
        if ((ccmd->input = malloc((e - s + 1) * sizeof(char))) == NULL)
            perror("tsh: findInput malloc");
        memcpy(ccmd->input, line + s, e - s + 1);
        ccmd->input[e - s] = '\0';
        for (size_t i = s; i < e; i++)
        {
            line[i] = ' ';
        }
    }
}

/**
 * @brief Find output path from a string and add it to the ComplexCommand_t if found
 * If the input char '>' is found, the next word is added to the command and removed from the string.
 * if ">>" is found, the field appendOut in cmd will be set to 1.
 * If nothing is found, an empty string will be added to cmd
 * @param line String to analyse.
 * @param ccmd ComplexCommand_t  where the string will be stored.
 */
void findOutput(char *line, ComplexCommand_t *ccmd)
{
    int s = 0;
    int e = 0;
    for (size_t i = 0; i < strlen(line); i++)
    {
        if (line[i] == '>')
        {
            if (i > 0 && line[i - 1] == '2')
            {
                continue;
            }
            if (line[i + 1] == '>')
            {
                ccmd->appendOut = 1;
                line[i + 1] = ' ';
            }
            for (size_t j = i + 1; j < strlen(line); j++)
            {
                if (line[j] != ' ')
                {
                    s = j;
                    break;
                }
            }
            //if no start then remove input char and break;
            if (s == 0)
            {
                line[i] = ' ';
                break;
            }
            //find end of input path
            for (size_t j = s; j < strlen(line) + 1; j++)
            {
                if (line[j] == ' ' || line[j] == '\0')
                {
                    e = j;
                    break;
                }
            }
            line[i] = ' ';
            break;
        }
    }
    if (s == 0)
    {
        ccmd->output = malloc(1);
        ccmd->output[0] = '\0';
    }
    else
    {
        if ((ccmd->output = malloc((e - s + 1) * sizeof(char))) == NULL)
            perror("tsh: findOutput malloc");
        memcpy(ccmd->output, line + s, e - s + 1);
        ccmd->output[e - s] = '\0';
        for (size_t i = s; i < e; i++)
        {
            line[i] = ' ';
        }
    }
}

/**
 * @brief Find err path from a string and add it to the ComplexCommand_t if found
 * If the input string "2>" is found, the next word is added to the command and removed from the string.
 * if "2>>" is found, the field appendErr in cmd will be set to 1.
 * If nothing is found, an empty string will be added to cmd
 * @param line String to analyse.
 * @param ccmd ComplexCommand_t  where the string will be stored.
 */
void findErr(char *line, ComplexCommand_t *ccmd)
{
    int s = 0;
    int e = 0;
    for (size_t i = 1; i < strlen(line); i++)
    {
        if (line[i] == '>' && line[i - 1] == '2')
        {
            if (line[i + 1] == '>')
            {
                ccmd->appendErr = 1;
                line[i + 1] = ' ';
            }
            for (size_t j = i + 1; j < strlen(line); j++)
            {
                if (line[j] != ' ')
                {
                    s = j;
                    break;
                }
            }
            //if no start then remove input char and break;
            if (s == 0)
            {
                line[i] = ' ';
                line[i - 1] = ' ';
                break;
            }
            //find end of input path
            for (size_t j = s; j < strlen(line) + 1; j++)
            {
                if (line[j] == ' ' || line[j] == '\0')
                {
                    e = j;
                    break;
                }
            }
            line[i - 1] = ' ';
            line[i] = ' ';
            break;
        }
    }
    if (s == 0)
    {
        ccmd->err = malloc(1);
        ccmd->err[0] = '\0';
    }
    else
    {
        if ((ccmd->err = malloc((e - s + 1) * sizeof(char))) == NULL)
            perror("tsh: findErr malloc");
        memcpy(ccmd->err, line + s, e - s + 1);
        ccmd->err[e - s] = '\0';
        for (size_t i = s; i < e; i++)
        {
            line[i] = ' ';
        }
    }
}

/**
 * @brief Parse the line into a null-terminated array of SimpleCommand_t and add it to a ComplexCommand_t struct. 
 * Also find the input, output and err path for redirection if they exist.
 * @param line Line to parse
 * @return a ComplexCommand_t struct
*/
ComplexCommand_t *parse_line(char *line)
{
    int simpCmdArraySize = NBARGS;
    ComplexCommand_t *cmd = malloc(sizeof(ComplexCommand_t));
    char **simpcmdtoparse = malloc(simpCmdArraySize * sizeof(char *));

    if (cmd == NULL || simpcmdtoparse == NULL)
        perror("tsh: parse_line malloc");

    findInput(line, cmd);
    findErr(line, cmd);
    findOutput(line, cmd);

    //parsing simple cmds
    char *cmdString = strtok(line, "|");
    int nbcmd = 0;
    while (cmdString != NULL)
    {
        if (nbcmd == simpCmdArraySize)
        {
            simpCmdArraySize += NBARGS;
            if ((simpcmdtoparse = realloc(simpcmdtoparse, simpCmdArraySize * sizeof(char *))) == NULL)
                perror("tsh: parse_line realloc");
        }
        if ((simpcmdtoparse[nbcmd] = malloc((strlen(cmdString) + 1) * sizeof(char))) == NULL)
            perror("tsh: parse_simpCmd malloc3");
        memcpy(simpcmdtoparse[nbcmd], cmdString, strlen(cmdString) + 1);
        nbcmd++;
        cmdString = strtok(NULL, "|");
    }
    SimpleCommand_t **simpCmds = malloc(nbcmd * sizeof(SimpleCommand_t *));
    if (simpCmds == NULL)
        perror("tsh: parse_line malloc");
    for (size_t i = 0; i < nbcmd; i++)
    {
        simpCmds[i] = parse_simpCmd(simpcmdtoparse[i]);
    }

    cmd->simpCmds = simpCmds;
    cmd->nbcmd = nbcmd;
    return cmd;
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
    if (args == NULL || cmd == NULL || ops == NULL)
        perror("tsh: parse_simpCmd malloc1");

    char *word = strtok(line, " \n\t");

    int nbword = 0;
    int nbops = 0;
    while (word != NULL)
    {

        if (nbword == nbargs)
        {
            nbargs += NBARGS;
            if ((args = realloc(args, nbargs * sizeof(char *))) == NULL)
                perror("tsh: parse_simpCmd realloc1");
        }

        if (word[0] == '-' && strlen(word) != 1)
        {
            if ((ops[nbops] = malloc(strlen(word) + 1)) == NULL)
                perror("tsh: parse_simpCmd malloc2");
            memcpy(ops[nbops], word, strlen(word) + 1);
            nbops++;
            if ((ops = realloc(ops, (nbops + 1) * sizeof(int))) == NULL)
                perror("tsh: parse_simpCmd realloc2");
        }

        if ((args[nbword] = malloc((strlen(word) + 1) * sizeof(char))) == NULL)
            perror("tsh: parse_simpCmd malloc3");
        memcpy(args[nbword], word, strlen(word) + 1);
        if (word[strlen(word) - 1] == '\\')
        {
            int newsize = strlen(word) + 1;
            word = strtok(NULL, " \n\t");
            newsize += strlen(word) + 1;
            if ((args[nbword] = realloc(args[nbword], newsize)) == NULL)
                perror("tsh: parse_simpCmd realloc3");
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
        perror("tsh: call_existing_command fork");
    }
    else if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("tsh: call_existing_command execvp failed");
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
 * @brief Execute a Simple command. If the command is builtin then call the function to execute it. If it isn't it will try to find the command 
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
    for (size_t i = 0; i < 10; i++)
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
        write(STDERR_FILENO, "tsh: can't call ", strlen("tsh: can't call "));
        write(STDERR_FILENO, cmd->args[0], strlen(cmd->args[0]));
        write(STDERR_FILENO, " in a tar. First use cd to get out.\n", strlen(" in a tar. First use cd to get out.\n"));
        return -1;
    }
}

/**
 * @brief Execute a ComplexCommand_t
 * 
 * @param cmd Command to execute
 * @return 1 if successful, -1 if an error was caught 
 */
int exec_complexcmd(ComplexCommand_t *cmd)
{
    int fdin, fdout, fderr, status, retval;
    retval = 1;
    pid_t cpid, wpid;
    //save current file descriptor
    int tmpin = dup(STDIN_FILENO);
    int tmpout = dup(STDOUT_FILENO);
    int tmperr = dup(STDERR_FILENO);

    char *path_inputStruct = malloc(strlen(cmd->input) + 1);
    memcpy(path_inputStruct, cmd->input, strlen(cmd->input) + 1);
    pathStruct *true_input = makeStructFromPath(path_inputStruct);
    if (strcmp(cmd->input, "") == 0) // if there isn't an input redirection
    {
        fdin = dup(tmpin);
    }
    else
    {
        if (true_input->isTarIndicated) //if the input redirection is a file in a tar
        {
            char *tmp_input_path = malloc(strlen("/tmp/tsh_tmp_in") + 1);
            strcpy(tmp_input_path, "/tmp/tsh_tmp_in");
            pathStruct *tmp_input = makeStructFromPath(tmp_input_path);
            cpTar(true_input, tmp_input, 0, true_input->name); //copy the file into a tmp file
            freeStruct(tmp_input);
            free(tmp_input_path);
            if ((fdin = open("/tmp/tsh_tmp_in", O_RDONLY)) == -1)
                perror("tsh: open");
        }
        else // if not open the file
        {
            if ((fdin = open(cmd->input, O_RDONLY, S_IRUSR)) == -1)
                perror("tsh : open exec_complexcmd");
        }
    }
    freeStruct(true_input);

    char *path_errStruct = malloc(strlen(cmd->err) + 1);
    memcpy(path_errStruct, cmd->err, strlen(cmd->err) + 1);
    pathStruct *true_err = makeStructFromPath(path_errStruct);
    if (strcmp(cmd->err, "") == 0) //if no error redirection has been given
    {
        fderr = dup(tmperr);
    }
    else
    {
        if (true_err->isTarIndicated) // if the error redirection is in a tar
        {
            if (cmd->appendErr)
            {
                char *path_src_err = malloc(strlen("/tmp/tsh_tmp_err") + 1);
                strcpy(path_src_err, "/tmp/tsh_tmp_err");
                pathStruct *tmp_err = makeStructFromPath(path_src_err);
                cpTar(true_err, tmp_err, 0, true_err->name); //copy the file in the tar we want to append data out of the tar
                rm_in_tar(true_err, 0);                      //delete the file in the tar (we will copy it back inside)
                freeStruct(tmp_err);
                if ((fderr = open("/tmp/tsh_tmp_err", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1) //open a blank tmp file
                    perror("tsh: open");
            }
            else
            {
                if ((fderr = open("/tmp/tsh_tmp_err", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1) //open a blank tmp file
                    perror("tsh: open");
            }
        }
        else
        {
            if (cmd->appendErr)
            {
                if ((fderr = open(cmd->err, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1)
                    perror("tsh : open exec_complexcmd");
            }
            else
            {
                if ((fderr = open(cmd->err, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
                    perror("tsh : open exec_complexcmd");
            }
        }
    }
    dup2(fderr, STDERR_FILENO);
    close(fderr);
    //if there's only one simple command
    if (cmd->nbcmd == 1)
    {
        char *path_outStruct = malloc(strlen(cmd->output) + 1);
        memcpy(path_outStruct, cmd->output, strlen(cmd->output) + 1);
        pathStruct *true_output = makeStructFromPath(path_outStruct);
        //Change err and std output
        if (strcmp(cmd->output, "") == 0) // if no output redirection has been found
        {
            fdout = dup(tmpout);
        }
        else
        {

            if (true_output->isTarIndicated) // if the output redirection is in a tar
            {
                if (cmd->appendOut)
                {
                    char *path_src_out = malloc(strlen("/tmp/tsh_tmp_out") + 1);
                    strcpy(path_src_out, "/tmp/tsh_tmp_out");
                    pathStruct *tmp_output = makeStructFromPath(path_src_out);
                    cpTar(true_output, tmp_output, 0, true_output->name); //copy the file in the tar we want to append data out of the tar
                    rm_in_tar(true_output, 0);                            //delete the file in the tar (we will copy it back inside)
                    freeStruct(tmp_output);
                    if ((fdout = open("/tmp/tsh_tmp_out", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1) //open a blank tmp file
                        perror("tsh: open");
                }
                else
                {
                    if ((fdout = open("/tmp/tsh_tmp_out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
                        perror("tsh: open");
                }
            }
            else
            {
                if (cmd->appendOut)
                {
                    if ((fdout = open(cmd->output, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1)
                        perror("tsh : open exec_complexcmd");
                }
                else
                {
                    if ((fdout = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
                        perror("tsh : open exec_complexcmd");
                }
            }
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

        if (strcmp(cmd->output, "") != 0 && true_output->isTarIndicated) //if the output redirection was in a tar (tmp file was opened)
        {
            char *path_src_out = malloc(strlen("/tmp/tsh_tmp_out") + 1);
            strcpy(path_src_out, "/tmp/tsh_tmp_out");
            pathStruct *tmp_output = makeStructFromPath(path_src_out); //turn a const string into a string
            cpTar(tmp_output, true_output, 0, tmp_output->name);       //copy the content of the tmp file into the tar at the correct location
            freeStruct(tmp_output);
        }
        freeStruct(true_output);

        if (strcmp(cmd->err, "") != 0 && true_err->isTarIndicated) //if the error redirection was in a tar (tmp file was opened)
        {
            char *path_src_err = malloc(strlen("/tmp/tsh_tmp_err") + 1);
            strcpy(path_src_err, "/tmp/tsh_tmp_err");
            pathStruct *tmp_err = makeStructFromPath(path_src_err);
            cpTar(tmp_err, true_err, 0, tmp_err->name);
            freeStruct(tmp_err);
        }
        freeStruct(true_err);

        return retval;
    }
    //if there is more than one simple command
    char *path_outStruct = malloc(strlen(cmd->output) + 1);
    memcpy(path_outStruct, cmd->output, strlen(cmd->output) + 1);
    pathStruct *true_output = makeStructFromPath(path_outStruct);
    for (size_t i = 0; i < cmd->nbcmd; i++)
    {

        dup2(fdin, STDIN_FILENO); //in the first loop, set the input of the first command to be fdin
        close(fdin);              // in the following loops, set the input to be from a pipe
        if (i == cmd->nbcmd - 1)  // if we reached the last command, set the output to a file
        {
            if (strcmp(cmd->output, "") == 0)
            {
                fdout = dup(tmpout);
            }
            else
            {
                //Change err and std output
                if (strcmp(cmd->output, "") == 0) // if no output redirection has been found
                {
                    fdout = dup(tmpout);
                }
                else
                {
                    if (true_output->isTarIndicated) // if the output redirection is in a tar
                    {
                        if (cmd->appendOut)
                        {
                            char *path_src_out = malloc(strlen("/tmp/tsh_tmp_out") + 1);
                            strcpy(path_src_out, "/tmp/tsh_tmp_out");
                            pathStruct *tmp_output = makeStructFromPath(path_src_out);
                            cpTar(true_output, tmp_output, 0, true_output->name); //copy the file in the tar we want to append data out of the tar
                            rm_in_tar(true_output, 0);                            //delete the file in the tar (we will copy it back inside)
                            freeStruct(tmp_output);
                            if ((fdout = open("/tmp/tsh_tmp_out", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1) //open a blank tmp file
                                perror("tsh: open");
                        }
                        else
                        {
                            if ((fdout = open("/tmp/tsh_tmp_out", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
                                perror("tsh: open");
                        }
                    }
                    else
                    {
                        if (cmd->appendOut)
                        {
                            if ((fdout = open(cmd->output, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR)) == -1)
                                perror("tsh : open exec_complexcmd");
                        }
                        else
                        {
                            if ((fdout = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
                                perror("tsh : open exec_complexcmd");
                        }
                    }
                }
            }
        }
        else //if we havent reached the last command yet, set the intput and output to a pipe
        {
            int fdpipe[2];
            if (pipe(fdpipe) == -1)
                perror("tsh: exec_complexcmd pipe");
            fdin = fdpipe[0];
            fdout = fdpipe[1];
        }
        dup2(fdout, STDOUT_FILENO); // set the output of the command to be written in the pipe
        close(fdout);

        cpid = fork();
        if (cpid == -1)
            perror("tsh: exec_complexcmd fork");
        if (cpid == 0)
        {
            if (exec_cmd(cmd->simpCmds[i]) == 1)
            {
                exit(EXIT_SUCCESS);
            }
            else
            {
                printMessageTsh(1, "exec simple command failed");
                retval = -1;
                exit(EXIT_FAILURE);
            }
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

    if (strcmp(cmd->output, "") != 0 && true_output->isTarIndicated) //if the output redirection was in a tar (tmp file was opened)
    {
        char *path_src_out = malloc(strlen("/tmp/tsh_tmp_out") + 1);
        strcpy(path_src_out, "/tmp/tsh_tmp_out");
        pathStruct *tmp_output = makeStructFromPath(path_src_out); //turn a const string into a string
        cpTar(tmp_output, true_output, 0, tmp_output->name);       //copy the content of the tmp file into the tar at the correct location
        freeStruct(tmp_output);
    }
    freeStruct(true_output);

    if (strcmp(cmd->err, "") != 0 && true_err->isTarIndicated) //if the error redirection was in a tar (tmp file was opened)
    {
        char *path_src_err = malloc(strlen("/tmp/tsh_tmp_err") + 1);
        strcpy(path_src_err, "/tmp/tsh_tmp_err");
        pathStruct *tmp_err = makeStructFromPath(path_src_err);
        cpTar(tmp_err, true_err, 0, tmp_err->name);
        freeStruct(tmp_err);
    }
    freeStruct(true_err);

    return retval;
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
        perror("tsh: parse_path malloc1");
    char *word = strtok(path, "/");

    int nbword = 0;
    while (word != NULL)
    {

        if (nbword == nbargs)
        {
            nbargs += NBARGS;
            if ((args = realloc(args, nbargs * sizeof(char *))) == NULL)
                perror("tsh: realloc parse path");
        }

        if ((args[nbword] = malloc((strlen(word) + 1) * sizeof(char))) == NULL)
            perror("tsh: malloc parse path");
        memcpy(args[nbword], word, strlen(word) + 1);

        nbword++;
        word = strtok(NULL, "/");
    }
    args[nbword] = NULL;
    return args;
}

/**
 * @brief Execute the command "cd" from a path
 * 
 * @param path path to the destination.
 * @return 1 on success, -1 on failure 
 */
int cd_tar(char *path)
{
    //in case there is an error we can go back
    int pwdlen = PWDLEN;
    char *pwdtmp;
    if ((pwdtmp = malloc(pwdlen)) == NULL)
        perror("tsh: cd malloc");
    while (getcwd(pwdtmp, pwdlen) == NULL)
    {
        pwdlen *= 2;
        free(pwdtmp);
        if ((pwdtmp = malloc(pwdlen)) == NULL)
            perror("tsh: cd malloc");
    }
    int tmpDepth = tarDepth;
    char **tmpTarDir;
    if ((tmpTarDir = malloc((tarDepth + 2) * sizeof(char *))) == NULL)
        perror("tsh: cd malloc");
    for (size_t i = 0; i < tarDepth + 1; i++)
    {
        tmpTarDir[i] = malloc(strlen(tarDirArray[i]));
        memcpy(tmpTarDir[i], tarDirArray[i], strlen(tarDirArray[i]) + 1);
    }
    tmpTarDir[tarDepth + 1] = NULL;

    int failure = 0;

    int i = 0;
    //if the path start by '/'
    if (path[0] == '/')
    {
        if (chdir("/") != 0)
        {
            perror("tsh: cd chdir");
        }
    }
    //if the parth start with a ~
    if (path[0] == '~')
    {
        char *home = getenv("HOME");
        if (chdir(home) != 0)
        {
            perror("tsh: cd chdir");
        }
        i++;
    }
    char *chemin;
    if ((chemin = malloc(strlen(path) + 1)) == NULL)
        perror("tsh: cd malloc");
    memcpy(chemin, path, strlen(path) + 1);
    char **arrayDir = parse_path(chemin); //parsing th path into array of folder/file name
    while (arrayDir[i] != NULL)
    {
        //if the destination is a tar we add the file name into tarDirArray
        remove_escape_char(arrayDir[i]);
        if (isTar(arrayDir[i]))
        {
            tarDepth++;
            if ((fdTar = open(arrayDir[i], O_RDWR, S_IRUSR | S_IWUSR)) == -1)
            {
                tarDepth--;
                perror("tsh: cd");
                failure = 1;
                break;
            }
            if ((tarDirArray = malloc((tarDepth + 2) * sizeof(char *))) == NULL)
            {
                perror("tsh: cd");
                failure = 1;
                break;
            }
            if ((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL)
            {
                perror("tsh: cd");
                failure = 1;
                break;
            }
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
                        {
                            perror("tsh: cd");
                            failure = 1;
                            break;
                        }
                        tarDirArray[tarDepth + 1] = NULL;
                    }
                } //if we go down in the tar
                else if (strcmp(arrayDir[i], ".") != 0)
                {
                    //if it is a folder connected from where we are in the tar
                    if (isTarFolder(arrayDir[i], tarDirArray) == 1)
                    {
                        tarDepth++; //increment tardepth to add a new folder
                        if ((tarDirArray = realloc(tarDirArray, (tarDepth + 2) * sizeof(char *))) == NULL)
                        { //realloc one size bigger
                            perror("tsh: cd");
                            failure = 1;
                            break;
                        }
                        //malloc the space required for the name of the new folder
                        if ((tarDirArray[tarDepth] = malloc((strlen(arrayDir[i]) + 1) * sizeof(char))) == NULL)
                        {
                            perror("tsh: cd");
                            failure = 1;
                            break;
                        }
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
    //if at one point we encounter an error, go back to the original position
    if (failure)
    {
        chdir(pwdtmp);
        tarDirArray = malloc((tmpDepth + 2) * sizeof(char *));
        for (size_t i = 0; i < tmpDepth + 1; i++)
        {
            tarDirArray[i] = malloc(strlen(tmpTarDir[i]));
            memcpy(tarDirArray[i], tmpTarDir[i], strlen(tmpTarDir[i]) + 1);
        }
        tarDirArray[tmpDepth + 1] = NULL;
        tarDepth = tmpDepth;
        for (size_t i = 0; tmpTarDir[i] == NULL; i++)
        {
            free(tmpTarDir[i]);
        }
        free(tmpTarDir);
        return -1;
    }
    free(pwdtmp);
    for (size_t i = 0; tmpTarDir[i] == NULL; i++)
    {
        free(tmpTarDir[i]);
    }
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
 * @brief Execute the command "cat".
 * 
 * @param cmd Command to execute. Contains the path and options
 * @return 1 on success 
 */
int tsh_cat(SimpleCommand_t *cmd)
{
    if ((cmd->nbargs - cmd->nb_options) == 1)
        call_existing_command(cmd->args);
    else
    {
        for (size_t i = 1; i < cmd->nbargs; i++)
        {
            if (!is_an_option(cmd->args[i]))
            {
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
                    //get the path to ls inside the tar
                    char *path_in_tar = array_to_path(abs_path_split[2], 0);
                    cat(path_to_open, path_in_tar);
                    free(path_to_open);
                    free(path_in_tar);
                }
                else
                {
                    //get the path to cat
                    char *path_to_cat = array_to_path(abs_path_array, 1);
                    if (strcmp(path_to_cat, "") == 0)
                        path_to_cat[0] = '/';
                    //allocate memory for "cat", the options, and the path
                    char **args = malloc((cmd->nb_options + 3) * sizeof(char *));
                    args[0] = malloc(strlen(cmd->args[0]) + 1);
                    memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                    for (size_t i = 0; i < cmd->nb_options; i++)
                    {
                        args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                        memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                    }
                    args[cmd->nb_options + 1] = malloc(strlen(path_to_cat) + 1);
                    memcpy(args[cmd->nb_options + 1], path_to_cat, strlen(path_to_cat) + 1);
                    args[cmd->nb_options + 2] = NULL;
                    //write(1, path_to_ls, strlen(path_to_ls));
                    free(path_to_cat);

                    call_existing_command(args);
                    for (size_t i = 0; i < cmd->nb_options + 3; i++)
                    {
                        free(args[i]);
                    }
                    free(args);
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
                write(STDOUT_FILENO, cmd->args[i], strlen(cmd->args[i])); //write the name of the path to ls
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
                    for (size_t i = 0; i < cmd->nb_options + 3; i++)
                    {
                        free(args[i]);
                    }
                    free(args);
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

    write(STDOUT_FILENO, pwd, strlen(pwd));
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

/**
 * @brief Execute the command "cp".  
 * 
 * @param cmd Command to execute. Contains the path and options
 * @return 1 on success, -1 on failure
 */
int tsh_cp(SimpleCommand_t *cmd)
{
    if (cmd->nbargs - cmd->nb_options < 3)
    {
        printMessageTsh(STDERR_FILENO, "Il faut 3 arguments pour la fonction cp");
        return -1;
    }
    int opt = has_correct_option(cmd->options, "-r");
    pathStruct *pathDest = makeStructFromPath(cmd->args[cmd->nbargs - 1]);
    int dest_exist = 0;
    if (isTar(pathDest->path))
        dest_exist = doesTarExist(pathDest->path);
    for (size_t i = 1; i < cmd->nbargs - 1; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            pathStruct *pathSrc = makeStructFromPath(cmd->args[i]);
            if (pathSrc->isTarIndicated && !pathSrc->isTarBrowsed && pathDest->isTarIndicated && !pathDest->isTarBrowsed && !dest_exist && (cmd->nbargs - cmd->nb_options) == 3)
            { //if we want to cp a tar to another tar that doesn't exist (rename), call cp
                if (!opt)
                {
                    write(STDERR_FILENO, "tsh: cp: -r not specified; ommitting directory '", strlen("tsh: cp: -r not specified; ommitting directory '"));
                    write(STDERR_FILENO, cmd->args[i], strlen(cmd->args[i]));
                    write(STDERR_FILENO, "'\n", strlen("'\n"));
                }
                else
                {
                    //allocate memory for "cp": cp, and the path_src, the path_dst, and NULL
                    char **args = malloc(4 * sizeof(char *));
                    args[0] = malloc(strlen(cmd->args[0]) + 1);
                    memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);

                    args[1] = malloc(strlen(pathSrc->path) + 1);
                    memcpy(args[1], pathSrc->path, strlen(pathSrc->path) + 1);
                    args[2] = malloc(strlen(pathDest->path) + 1);
                    memcpy(args[2], pathDest->path, strlen(pathDest->path) + 1);
                    args[3] = NULL;

                    call_existing_command(args);
                    for (size_t i = 0; i < 4; i++)
                    {
                        free(args[i]);
                    }
                    free(args);
                }
            }
            else if (pathSrc->isTarBrowsed || pathDest->isTarIndicated) //if the file/folder we want to copy is a tar or go through a tar
            {
                if (cpTar(pathSrc, pathDest, opt, pathSrc->name) == -1)
                {
                    freeStruct(pathSrc);
                    freeStruct(pathDest);
                    return -1;
                }
            }
            else
            {
                //allocate memory for "cp": cp, the options, and the path_src, the path_dst
                char **args = malloc((cmd->nb_options + 4) * sizeof(char *));
                args[0] = malloc(strlen(cmd->args[0]) + 1);
                memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                for (size_t i = 0; i < cmd->nb_options; i++)
                {
                    args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                    memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                }
                args[cmd->nb_options + 1] = malloc(strlen(pathSrc->path) + 1);
                memcpy(args[cmd->nb_options + 1], pathSrc->path, strlen(pathSrc->path) + 1);
                args[cmd->nb_options + 2] = malloc(strlen(pathDest->path) + 1);
                memcpy(args[cmd->nb_options + 2], pathDest->path, strlen(pathDest->path) + 1);
                args[cmd->nb_options + 3] = NULL;

                call_existing_command(args);
                for (size_t i = 0; i < cmd->nb_options + 4; i++)
                {
                    free(args[i]);
                }
                free(args);
            }
            freeStruct(pathSrc);
        }
    }
    freeStruct(pathDest);
    return 1;
}

/**
 * @brief Execute the command "rm".  
 * 
 * @param cmd Command to execute. Contains the path and options
 * @return 1 on success, -1 on failure
 */
int tsh_rm(SimpleCommand_t *cmd)
{
    if (cmd->nbargs - cmd->nb_options < 2)
    {
        printMessageTsh(STDERR_FILENO, "Il faut 2 arguments pour la fonction rm");
        return -1;
    }

    int opt = has_correct_option(cmd->options, "-r");
    for (size_t i = 1; i < cmd->nbargs; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            pathStruct *pathSrc = makeStructFromPath(cmd->args[i]);
            if (pathSrc->isTarBrowsed) //if the file/folder we want to copy is a tar or go through a tar
            {
                if (rm_in_tar(pathSrc, opt) == -1)
                {
                    freeStruct(pathSrc);
                    return -1;
                }
            }
            else if (pathSrc->isTarIndicated && opt != 1)
            {
                printMessageTsh(STDERR_FILENO, "Pour supprimer un dossier, veuillez utiliser l'option -r ou la commande rmdir");
                freeStruct(pathSrc);
                return -1;
            }
            else
            {
                //allocate memory for "cp": cp, the options, and the path_src
                char **args = malloc((cmd->nb_options + 3) * sizeof(char *));
                args[0] = malloc(strlen(cmd->args[0]) + 1);
                memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                for (size_t i = 0; i < cmd->nb_options; i++)
                {
                    args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                    memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                }
                args[cmd->nb_options + 1] = malloc(strlen(pathSrc->path) + 1);
                memcpy(args[cmd->nb_options + 1], pathSrc->path, strlen(pathSrc->path) + 1);
                args[cmd->nb_options + 2] = NULL;

                call_existing_command(args);
                for (size_t i = 0; i < cmd->nb_options + 3; i++)
                {
                    free(args[i]);
                }
                free(args);
            }
            freeStruct(pathSrc);
        }
    }
    return 1;
}

/**
 * @brief Execute the command "mkdir".  
 * 
 * @param cmd Command to execute. Contains the path and options
 * @return 1 on success, -1 on failure
 */
int tsh_mkdir(SimpleCommand_t *cmd)
{
    if (cmd->nbargs - cmd->nb_options < 1)
    {
        printMessageTsh(STDERR_FILENO, "Il faut au moins 1 argument pour la fonction mkdir");
        return -1;
    }

    for (size_t i = 1; i < cmd->nbargs; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            pathStruct *pathSrc = makeStructFromPath(cmd->args[i]);
            if (pathSrc->isTarBrowsed)
            {
                if (mkdirTar(pathSrc) == -1)
                {
                    freeStruct(pathSrc);
                    return -1;
                }
            }
            else if (pathSrc->isTarIndicated)
            {
                if (mkTarEmpty(pathSrc->path) == -1)
                {
                    freeStruct(pathSrc);
                    return -1;
                }
            }
            else
            {
                //allocate memory for "cp": cp, the options, and the path_src
                char **args = malloc((cmd->nb_options + 3) * sizeof(char *));
                args[0] = malloc(strlen(cmd->args[0]) + 1);
                memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                for (size_t i = 0; i < cmd->nb_options; i++)
                {
                    args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                    memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                }
                args[cmd->nb_options + 1] = malloc(strlen(pathSrc->path) + 1);
                memcpy(args[cmd->nb_options + 1], pathSrc->path, strlen(pathSrc->path) + 1);
                args[cmd->nb_options + 2] = NULL;

                call_existing_command(args);
                for (size_t i = 0; i < cmd->nb_options + 3; i++)
                {
                    free(args[i]);
                }
                free(args);
            }
            freeStruct(pathSrc);
        }
    }
    return 1;
}

int tsh_rmdir(SimpleCommand_t *cmd)
{
    if (cmd->nbargs - cmd->nb_options < 2)
    {
        printMessageTsh(STDERR_FILENO, "Il faut 2 arguments pour la fonction rm");
        return -1;
    }

    for (size_t i = 1; i < cmd->nbargs; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            pathStruct *pathSrc = makeStructFromPath(cmd->args[i]);
            if (pathSrc->isTarBrowsed)
            {
                if (rmdirInTar(pathSrc) == -1)
                {
                    freeStruct(pathSrc);
                    return -1;
                }
            }
            else if (pathSrc->isTarIndicated)
            {
                if (rmdirTar(pathSrc->path) == -1)
                {
                    freeStruct(pathSrc);
                    return -1;
                }
            }
            else
            {
                //allocate memory for "cp": cp, the options, and the path_src
                char **args = malloc((cmd->nb_options + 3) * sizeof(char *));
                args[0] = malloc(strlen(cmd->args[0]) + 1);
                memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                for (size_t i = 0; i < cmd->nb_options; i++)
                {
                    args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                    memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                }
                args[cmd->nb_options + 1] = malloc(strlen(pathSrc->path) + 1);
                memcpy(args[cmd->nb_options + 1], pathSrc->path, strlen(pathSrc->path) + 1);
                args[cmd->nb_options + 2] = NULL;

                call_existing_command(args);
                for (size_t i = 0; i < cmd->nb_options + 3; i++)
                {
                    free(args[i]);
                }
                free(args);
            }
            freeStruct(pathSrc);
        }
    }
    return 1;
}

int tsh_mv(SimpleCommand_t *cmd)
{
    if (cmd->nbargs - cmd->nb_options < 3)
    {
        printMessageTsh(STDERR_FILENO, "Il faut 3 arguments pour la fonction cp");
        return -1;
    }

    pathStruct *pathDest = makeStructFromPath(cmd->args[cmd->nbargs - 1]);
    int isDirDest = isADirectory(pathDest);

    int nbSources = 0;
    for (size_t i = 1; i < cmd->nbargs - 1; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            if (nbSources == 1)
            {
                printMessageTsh(STDERR_FILENO, "Le dernier argument n'est pas un dossier");
                freeStruct(pathDest);
                return -1;
            }
            nbSources++;
        }
    }

    for (size_t i = 1; i < cmd->nbargs - 1; i++)
    {
        if (!is_an_option(cmd->args[i]))
        {
            pathStruct *pathSrc = makeStructFromPath(cmd->args[i]);
            if (pathSrc->isTarIndicated || pathDest->isTarIndicated) //if the file/folder we want to move or rename is a tar or go through a tar
            {
                if (mvWithTar(pathSrc, pathDest) == -1)
                {
                    freeStruct(pathSrc);
                    freeStruct(pathDest);
                    return -1;
                }
            }
            else
            {
                //allocate memory for "cp": cp, the options, and the path_src, the path_dst
                char **args = malloc((cmd->nb_options + 4) * sizeof(char *));
                args[0] = malloc(strlen(cmd->args[0]) + 1);
                memcpy(args[0], cmd->args[0], strlen(cmd->args[0]) + 1);
                for (size_t i = 0; i < cmd->nb_options; i++)
                {
                    args[i + 1] = malloc(strlen(cmd->options[i]) + 1);
                    memcpy(args[i + 1], cmd->options[i], strlen(cmd->options[i]) + 1);
                }
                args[cmd->nb_options + 1] = malloc(strlen(pathSrc->path) + 1);
                memcpy(args[cmd->nb_options + 1], pathSrc->path, strlen(pathSrc->path) + 1);
                args[cmd->nb_options + 2] = malloc(strlen(pathDest->path) + 1);
                memcpy(args[cmd->nb_options + 2], pathDest->path, strlen(pathDest->path) + 1);
                args[cmd->nb_options + 3] = NULL;

                call_existing_command(args);
                for (size_t i = 0; i < cmd->nb_options + 4; i++)
                {
                    free(args[i]);
                }
                free(args);
            }
            freeStruct(pathSrc);
        }
    }
    freeStruct(pathDest);
    return 1;
}

/**
 * @brief Construct a pathStruct struct from a path.
 * 
 * @param path path to construct the struct from.
 * @return pathStruct pointer
 */
pathStruct *makeStructFromPath(char *path)
{
    char *pwd = get_pwd();
    char **pathToArr = parsePathAbsolute(path, pwd);
    free(pwd);
    char ***path3D = path_to_tar_file_path_new(pathToArr);

    pathStruct *pathRes = malloc(sizeof(pathStruct));

    if (path3D[1] == NULL)
    {
        pathRes->isTarBrowsed = 0;
        pathRes->isTarIndicated = 0;
        pathRes->path = array_to_path(path3D[0], 1);
        pathRes->nameInTar = NULL;
    }
    else
    {
        if (path3D[2][0] == NULL)
        {
            pathRes->isTarBrowsed = 0;
        }
        else
        {
            pathRes->isTarBrowsed = 1;
        }
        pathRes->isTarIndicated = 1;
        pathRes->path = concatPathName(array_to_path(path3D[0], 1), path3D[1][0]);
        pathRes->nameInTar = array_to_path(path3D[2], 0);
    }

    pathRes->name = getLast(pathToArr);

    int i = 0;
    freeArr3D(path3D);

    return pathRes;
}

/**
 * @brief find if an option is in an option list. Used when calling a command in a tar to see if an option 
 * that is implemented has been found in the args of the command
 * 
 * @param optionlist list of option given.
 * @param option option to find
 * @return 1 if the option was found, 0 else
 */
int has_correct_option(char **optionlist, const char *option)
{
    int i = 0;
    while (optionlist[i] != NULL)
    {
        if (strcmp(optionlist[i], option) == 0)
        {
            return 1;
        }
    }
    return 0;
}