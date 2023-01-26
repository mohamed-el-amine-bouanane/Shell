#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chemin.h"
#include "chemins.h"

#define CAP 10

/**
 * retourne un tableau de chemins de capacité CAP ; ou NULL en cas d'erreur
 */
chemins * init_chemins(){
    chemins * res = malloc(sizeof(chemins));
    if(res == NULL) goto error;
    res->capacity = CAP;
    res->length = 0;
    char ** tab = malloc(CAP * sizeof(char *));
    if(tab == NULL) goto error;
    res->tab = tab;
    return res;

    error:
    if(res) free(res);
    if(tab) free(tab);
    return NULL;
}

/**
 * ajoute chemin->data au tableau de chemins et renvoie le nouveau tableau;
 * ou NULL en cas d'erreur
 */
chemins * add_chemins (chemins* chemins, chemin* c){
    //tableau plein
    if(chemins->capacity == chemins->length) {
        char ** tmp = realloc (chemins->tab,2*chemins->capacity*sizeof(char*));
        if(tmp == NULL) goto error;
        chemins->tab = tmp;
        chemins->capacity*=2;
    }
    char* nouveau_chemin = malloc(c->length+1);
    if(nouveau_chemin == NULL) goto error;
    nouveau_chemin[c->length]='\0';
    memmove(nouveau_chemin, c->data, c->length);

    chemins->tab[chemins->length] = nouveau_chemin;
    chemins->length++;
    return chemins;
    
    error:
    return NULL;
}

void supprime_chemins(chemins* chemins){
    for(int i=0; i<chemins->length; i++) free(chemins->tab[i]);
    free(chemins->tab);
    free(chemins);
}

void free_chemins(chemins* chemins){
    free(chemins->tab);
    free(chemins);
}

void affichage_chemins (chemins * c){
    for(int i=0; i<c->length; i++){
        printf("%s\n" , c->tab[i]);
    }
}

char** assembler_chemins(chemins** tab_chemins, int nb_mots)
//prend un tableau de chemins et renvoie un tableau de tableau de charactere
{
    int count = 0;//nouveau nombre de mots
    for(int i=0; i<nb_mots; i++){
        count+=tab_chemins[i] -> length;
    }
    char** res = malloc((count+1)*sizeof(char*));//+1 pour le NULL à la fin
    if(res==NULL) goto error;
    int c=0;
    for(int i=0; i<nb_mots; i++){
        for(int j=0; j<tab_chemins[i]->length; j++){
            res[c] = tab_chemins[i]->tab[j];
            c++;
        }
    }
    res[count] = NULL;
    return res;
    error:
    return NULL;
}