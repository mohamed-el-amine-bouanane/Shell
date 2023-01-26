#ifndef CHEMINS_H
#define CHEMINS_H

#include "chemin.h"


typedef struct{
    int capacity;
    int length;
    char** tab;
}chemins;


void free_chemins(chemins*);
char ** assembler_chemins(chemins** ,int);
void affichage_chemins (chemins * c);
chemins * init_chemins();
chemins * add_chemins (chemins* chemins, chemin* c);
void supprime_chemins(chemins* cs);

#endif