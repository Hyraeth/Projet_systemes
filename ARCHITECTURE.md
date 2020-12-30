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
 ┣ tsh.c
 ┣ commandes
 ┃ ┣ cat_tar.c
 ┃ ┣ cp_tar.c
 ┃ ┣ ls_tar.c
 ┃ ┣ mkdir_tar.c
 ┃ ┣ mv_tar.c
 ┃ ┣ rmdir_tar.c
 ┃ ┣ rm_tar.c
 ┃ ┣ tar.c
 ┃ ┣ tar_fun.c
 ┃ ┣ tsh_fun.c
 ┃ headers
 ┃ ┣ cat_tar.h
 ┃ ┣ cp_tar.h
 ┃ ┣ ....

## Structure de données

A remplir

## Algorithmes implémmentés

### Algorithme de traitement des chemins

cet algorithme permet séparer le chemin avant le tar, du chemin restant, permettant de traiter les données indépendament.
De plus, il supprime les fichiers . et ..