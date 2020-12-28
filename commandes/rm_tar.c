#include "../headers/rm_tar.h"

int rm_in_tar (pathStruct *pathToDelete,int op) {
	char c = typeFile(pathToDelete->path,pathToDelete->nameInTar);
	if (c == '9') {
		printMessageTsh("Veuillez vérifier que le fichier que vous voulez supprimer existe bien");
		return -1;
	}

	if (op == 0) {
		if (c == '5') {
			printMessageTsh("Pour supprimer un dossier, veuillez utiliser l'option -r ou la commande rmdir");
			return -1;
		}
		return deleteFileInTar (pathToDelete->nameInTar, pathToDelete->path);
	}
	else {
		return rmWithOptionTar(pathToDelete->path,pathToDelete->nameInTar);
	}
}

int rm_tar (char *path) {
	if (isEmptyTar(path)) {

	}
	else {
		printMessageTsh("Le tar que vous voulez supprimer n'est pas vide");
	}
}