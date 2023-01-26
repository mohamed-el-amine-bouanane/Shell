#ifndef PWD_H
#define PWD_H


/**
 * renvoie une chaine de caractères contenant le chemin absolue
 * physique du répertoire courant; ou NULL en cas d'erreur;
 * l'utilisateur devra free la chaine de caractères renvoyée   
 */
char * my_getcwd ();

/**
 * affiche le chemin logique du répertoire courant si param == L;
 * le chemin physique si param == P
 * retourne 0 en cas de succes; 1 en cas d'échec
 */
int pwd(int param);

#endif