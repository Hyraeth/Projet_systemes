#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char **get_path_tar (char *path);
void print_file (char *path_file);
void print_file_in_tar (char *path_tar, char *path_file_in_tar);

int read_write () {

}

int read_all_files (SimpleCommand_t *cmd) {
    for (int i = 0; i < cmd->nbargs; i++) {
        char **path = get_path_tar(cmd->args[i]);
        if (path[0] == NULL) {

        }
        else {

        }
    }
}

void print_file_in_tar (char *path_tar, char *path_file_in_tar) {
    printf("%s/n",get_file_tar(path_tar,path_file_in_tar));
}

void print_file (char *path_file) {
    int fd = open(path_file,O_RDONLY);
    if (fd == -1 ) perror("cat");
    while ((int lus = read()))
}


/*
 * path : the path we are looking to
 * return two strings with the path to a tar if it exists or NULL and the path to the file within the tar or simply the path to the file
 */
char **get_path_tar (char *path) {
    char * strToken = strtok ( str, separators );
    while ( strToken != NULL ) {
        printf ( "%s\n", strToken );
        // On demande le token suivant.
        strToken = strtok ( NULL, separators );
    }
}

int cat (SimpleCommand_t *cmd) {
    if (cmd->nbargs < 1) {
        return read_write();
    }
    else {
        return read_all_files(cmd);
    }
}

