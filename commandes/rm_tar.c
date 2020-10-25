#include "../headers/rm_tar.h"
#include "../headers/tsh_fun.h"
#include "../headers/tar_fun.h"

int rm_tar (char ***path,int op) {
	if (op == 0) {
		char *pathInTar = array_to_path(path[2],0);
		int a = deleteFileInTar (pathInTar, path[1][0]);
		free(pathInTar);
		return a;
	}
}