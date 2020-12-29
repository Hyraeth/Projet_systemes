#ifndef LS_TAR_H
#define LS_TAR_H

#include "tar_fun.h"


bool s_is_in_string(char * string, char * s);
bool contain_char(char * s, char c);
bool contain_one_char(char * s, char c);
void print_space(int n);
void print_name_file(struct posix_header * header, char * path);
void print_name_rep(struct posix_header * header, char * path);
void print_type (struct posix_header * header);
void print_right(struct posix_header * header);
void print_nb_link(struct posix_header * header, char * path, int fd);
void print_size(struct posix_header * header, char * path, int fd);
void print_time(struct posix_header * header);
void print_ls_l (struct posix_header * header, char * path, int fd);
void print_header_name(char *op, struct posix_header * header, char * path, int fd);
int ls_tar(char *op, char *path, int fd);

#endif //LS_TAR_H
