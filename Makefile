CC=gcc
CFLAGS=-g

tsh : commandes/tar.c commandes/tar_fun.c tsh_fun.c commandes/ls_tar.c tsh.c commandes/cp_tar.c commandes/rm_tar.c

run : tsh
	./tsh