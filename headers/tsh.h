#ifndef TSH_H
#define TSH_H

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "tar_fun.h"
#include "ls_tar.h"
#include "cat_tar.h"
#include "tsh_fun.h"
#include "cp_tar.h"
#include "rm_tar.h"
#include "mkdir_tar.h"
#include "rmdir_tar.h"
#include "mv_tar.h"

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

#endif