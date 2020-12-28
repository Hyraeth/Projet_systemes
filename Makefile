CC=gcc
CFLAGS=-g

tsh : commandes/tar.c commandes/tar_fun.c commandes/tsh_fun.c commandes/ls_tar.c tsh.c commandes/cp_tar.c commandes/rm_tar.c commandes/cat_tar.c commandes/mkdir_tar.c commandes/rmdir_tar.c

run : tsh
	./tsh