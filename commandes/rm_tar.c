#include "../headers/rm_tar.h"

int rm_tar (pathStruct *pathToDelete,int op) {
	if (op == 0) {
		return deleteFileInTar (pathToDelete->nameInTar, pathToDelete->path);
	}
	else {
		return rmWithOptionTar(pathToDelete->path,pathToDelete->nameInTar);
	}
}