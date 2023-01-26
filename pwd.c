#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "chemin.h"
#include "pwd.h"

#define L 0
#define P 1

/**
 * renvoie une chaine de caractères contenant le chemin absolue
 * physique du répertoire courant; ou NULL en cas d'erreur;
 * l'utilisateur devra free la chaine de caractères renvoyée   
 */
char * my_getcwd (){
    char * chemin = NULL;
    int i=1;
    while(1){// tant que le tableau chemin n'est pas assez grand pour stocker le chemin
        chemin=malloc(PATH_MAX*i);
        if(chemin == NULL){
            perror("malloc"); goto error;
        }
        char* tmp = getcwd(chemin, PATH_MAX*i);
        if(tmp!=NULL){// on a réussi à avoir le chemin
            chemin=tmp; 
            break;
        }
        else if(tmp==NULL && errno==ERANGE){// le tableau chemin est petit
            free(chemin);
            chemin=NULL;
            i++;
        }
        else{// une autre erreur
            goto error;
        }
    }
    return chemin;
    
    error:
    if(chemin!=NULL) free(chemin);
    return NULL;
}

/**
 * affiche le chemin logique du répertoire courant si param == L;
 * le chemin physique si param == P
 * retourne 0 en cas de succes; 1 en cas d'échec
 */
int pwd(int param){
    if(param == P){// pwd -P
        char * chemin = my_getcwd();
        if(chemin == NULL){//erreur
            perror("pwd -P");
            goto error;
        }
        write(STDOUT_FILENO,chemin, strlen(chemin));
        write(STDOUT_FILENO, "\n", 1);
        free(chemin);
    }
    else if(param==L){//pwd -L, on affiche $PWD
        char * path = getenv("PWD");
        if(path==NULL){
            perror("pwd -L");
            goto error;
        }
        write(STDOUT_FILENO,path, strlen(path));
        write(STDOUT_FILENO, "\n", 1);
    }
    return 0;
    
    error:
    return 1;
}