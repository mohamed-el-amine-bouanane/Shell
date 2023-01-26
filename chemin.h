#ifndef CHEMIN_H
#define CHEMIN_H

typedef struct chemin{
    unsigned int capacity;//taille du buffer
    unsigned int length;//taille de la chaine sans compter le \0 terminal
    char * pred;//pointeur vers le dernier  
    char * data;//contenu du chemin avec un \0 terminal
}chemin;


chemin * init(char * chemin_init);
int add_chemin (chemin * chemin, char * dir_name);
void supprime_chemin(chemin * chemin);
int truncate_chemin(chemin * chemin);
void print_chemin(chemin * c);
chemin* traitement_chemin(chemin * chemin);
char* truncate_chemin_gauche(chemin * chemin);


#endif