#include "../headers/rm_tar.h"

int rm_in_tar(pathStruct *pathToDelete, int op)
{
	char c = typeFile(pathToDelete->path, pathToDelete->nameInTar);
	if (c == '9')
	{
		printMessageTsh(STDERR_FILENO, "tsh: rm: No such file");
		return -1;
	}

	if (op == 0)
	{
		if (c == '5')
		{
			printMessageTsh(STDERR_FILENO, "tsh: rm: Is a Directory: use either rmdir or the -r option to delete directories");
			return -1;
		}
		return deleteFileInTar(pathToDelete->nameInTar, pathToDelete->path);
	}
	else
	{
		return rmWithOptionTar(pathToDelete->path, pathToDelete->nameInTar);
	}
}

int rm_tar(char *path)
{
	if (isEmptyTar(path))
	{
	}
	else
	{
		printMessageTsh(STDERR_FILENO, "tsh: rm: Directory not empty");
	}
}