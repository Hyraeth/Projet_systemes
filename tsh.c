#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headers/str_to_arr.h"

#define BUFLEN 512
#define NBARGS 8

char *read_line();
char **parse_line(char *line);
int exec_cmd(char **args);

int main(int argc, char const *argv[])
{
    int run = 1;
    do {
        char *pwd = getcwd(NULL, 0);
        strcat(pwd, ">");
        write(STDOUT_FILENO, pwd, strlen(pwd));

        char *line;
        char **args;

        line = read_line();
        args = parse_line(line);
        run = exec_cmd(args);

        free(line);
        int i = 0;
        while(args[i] != NULL){
            free(args[i]);
            i++;
        }
        free(args[i]);
        free(args);
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
        perror("tsh: allocation failed.");
    }
    int n;
    int totalread = 0;
    while ((n = read(STDIN_FILENO, line+totalread, BUFLEN)) > 0) {
        totalread += n;
        //add more space
        if(n == BUFLEN) {
            bufsize += BUFLEN;
            if((line = realloc(line, bufsize)) == NULL)
                perror("tsh: allocation failed.");
        }
        //stop
        if(line[totalread-1] == '\n') {
            line[totalread-1] = '\0';
            break;
        }
    }
    //remove excess space
    if((line = realloc(line, totalread)) == NULL)
        perror("tsh: allocation failed.");
    //error handling
    if (n < 0)
        perror("tsh: read failed.");
    return line;
}


char **parse_line(char *line) {
    int nbargs = NBARGS;
    char **args = malloc(nbargs*sizeof(char *));
    char *word = strtok(line, " \n\t");
    int nbword = 0;
    while(word != NULL) {
        if (nbword == nbargs) {
            nbargs += NBARGS;
            if((args = realloc(args, nbargs*sizeof(char *))) == NULL)
                perror("tsh: allocation failed.");
        }
        if((args[nbword] = malloc((strlen(word)+1) * sizeof(char))) == NULL)
            perror("tsh: allocation failed.");
        memcpy(args[nbword], word, strlen(word)+1);
        nbword++;
        word = strtok(NULL, " \n\t");
    }
    if((args = realloc(args, (nbword+1)*sizeof(char *))) == NULL)
        perror("tsh: allocation failed.");
    args[nbword] = NULL;
    return args;
}