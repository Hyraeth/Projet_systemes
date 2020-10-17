#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFLEN 4

char *read_line();
char **parse_line(char *line);
int exec_cmd(char **args);

int main(int argc, char const *argv[])
{
    int run = 1;
    while(run) {
        char *pwd = getcwd(NULL, 0);
        strcat(pwd, ">");
        write(STDOUT_FILENO, pwd, strlen(pwd));

        char *line;

        //char **args;

        line = read_line();
        //args = parse_line(line);
        //run = exec_cmd(args);
        printf("%s\n", line);
        free(line);
        //free(args);
        free(pwd);
    }
    return 0;
}

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
    if (n < 0)
        perror("tsh: read failed.");
    return line;
}
