# Architecture

## Architecture logicielle

Notre texte est divisé en 2 parties : une partie sur les commandes qui s'applique dans les tar, et une partie plus générale.

### Partie 1 : Les Commandes dans les tar

Cette partie contient toutes les commandes executables depuis un terminale (cat, cp, ls, mkdir, mv, ....), mais applicable sur les fichier tar, en considèrent ces dernier comme des repertoires.
Toutes les commades sont rangées dans le répertoire "commandes" et chaque commande est mis dans un fichier différent de la forme "commande_tar.c".

cette partie comprend également le fichier "tar_fun.c" qui contient des fonctions utilisées dans les fichier ci-dessus.

### Partie 2 : Plus générale

Dans cette partie, nous avons tout ce qui se réfère à un shell de base, et qui vas aussi implémenter les commandes de la partie 1.

## Arbre des fichiers

Projet_Systemes

- tsh.c
- commandes :
- - cat_tar.c
- - cp_tar.c
- - ls_tar.c
- - mkdir_tar.c
- - mv_tar.c
- - rmdir_tar.c
- - rm_tar.c
- - tar.c
- - tar_fun.c
- - tsh_fun.c
- headers :
- - cat_tar.h
- - cp_tar.h
- - ....

## Structure de données

Pour ce programme, nous avons utilisé une structure appelé pathStruct.
Cette structure permet, en utilisant l'algorithme de traitement des chemins évoqués plus bas, de bien séparer les chemins donnés en arguments des commandes afin de pouvoir ensuite traiter chaque cas facilement à l'aide de booléens et de chaînes de caractères bien créées pour contenir d'un côté le chemin allant jusqu'au tar ou au fichier ou dossier qui n'est pas un tar dans le cas échéant, et le chemin qui va jusqu'au fichier ou dossier dans le tar si indiqué.
Enfin, on garde le nom dans la structure qui nous permet de réaliser cp facilement.

## Algorithmes implémmentés

### Algorithme de traitement des chemins

Cet algorithme permet séparer le chemin avant le tar, du chemin restant, permettant de traiter les données indépendament.
Ainsi, il est ensuite plus aisé de travailler sur la structure créée qui permet rapidement de savoir si on utilise un tar ou non.
De plus, il supprime les entrées . et .. qui peuvent être indiquées dans les chemins

### Algorithme de suppression de fichiers et dossiers dans un tar

Probablement un des algorithmes qui a pris le plus de temps à être finis, avec cp. Même si on sait assez facilement quels fichiers et dossiers supprimer grâce aux fonctions isSubFile, savoir comment supprimer les fichiers dans le tar était un peu plus difficile. Nous avons choisi la manière directe, c'est à dire supprimé le fichier en réécrivant chaque octet correspondant au fichier à 0 puis ensuite de bouger les données qui étaient écrites après le fichier à la place de ce dernier. Enfin, on tronque le fichier pour supprimer les données inutiles. Cette manière de procéder est probablement pas la meilleure et explique probablement la lenteur de rm lorsqu'on essaye de supprimer de gros fichiers.

### Algorithme de copie de fichiers et dossiers dans un tar

Contrairement à la suppression, la copie de fichiers dans un tar était assez facile en soit, il suffit juste de se placer juste avant les blocs de 0 à la fin d'un tar, puis d'écrire les donnnées nécessaires pour le fichier et ensuite de réécrire les blocs de 0. Cependant, tous les cas différents ont demandé beaucoup de temps, de tests, mais nous avons réussi à gérer ces derniers en utilisant une fonction pour copier les dossiers et une fonction plus générale pour copier n'importe quel type de fichier ainsi que de multiples fonctions auxiliaires