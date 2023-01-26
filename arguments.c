#include <stdlib.h>
#include <string.h>
#include "chemin.h"
#include "chemins.h"
#include "etoile.h"
#include "double_etoile.h"

int contient_etoile(char* mot){
    int debut_base_name=0;
    for(int i=0; i<strlen(mot); i++){
        if(mot[debut_base_name]=='*') return 1;
        if(mot[i]=='/') debut_base_name=i+1;
    }
    return 0;
}

int commence_par_double_etoile(char * mot){
    if(strlen(mot)<3) return 0;
    if(mot[0]=='*' && mot[1]=='*' && mot[2]=='/') return 1;
    return 0;
}

char ** traiter_arguments(char** mots, int nb_mots){
    if(nb_mots==0) return mots;
    chemin* mot = NULL;
    chemins** tab_chemins= malloc(nb_mots*sizeof(chemins*));
    if(tab_chemins==NULL) goto error;
    for(int i=0; i<nb_mots; i++){//initialisation à NULL pour la gestion d'erreurs
        tab_chemins[i] = NULL;
    }

    for(int i=0; i<nb_mots; i++){
        if(commence_par_double_etoile(mots[i])){
            tab_chemins[i] = expansion_double_etoile(mots[i]);
            if(tab_chemins[i]==NULL) goto error;
        }
        else if(contient_etoile(mots[i])){
            tab_chemins[i] = expansion_etoile(mots[i]);
            if(tab_chemins[i]==NULL) goto error;
        }
        else{
            tab_chemins[i] = init_chemins();
            if(tab_chemins[i]==NULL) goto error;
            mot = init(mots[i]);
            if(mot==NULL) goto error;
            add_chemins(tab_chemins[i], mot);
            supprime_chemin(mot);
            mot=NULL;
        }
    }
    char ** res = assembler_chemins(tab_chemins, nb_mots);
    if(res==NULL) goto error;
    for(int i=0; i<nb_mots; i++){
        if(tab_chemins[i]!=NULL) free_chemins(tab_chemins[i]);
    }
    free(tab_chemins);
    return res;

    error:
    if(tab_chemins!=NULL){
        for(int i=0; i<nb_mots; i++){
            if(tab_chemins[i]!=NULL) supprime_chemins(tab_chemins[i]);
        }
        free(tab_chemins);
    }
    if(mot!=NULL) supprime_chemin(mot);

    return NULL;
}