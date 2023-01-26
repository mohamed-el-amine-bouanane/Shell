#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "chemin.h"


/*
 * libère toutes les resources allouées 
*/
void supprime_chemin(chemin * chemin)
{
    if(chemin !=NULL)
    {
    free(chemin->data);
    free(chemin);
    }
}

/**
 * prend une chaine de caractères représentant un chemin absolu 
 * sans / à la fin
 * ex : /aa/bbb/cccc
 * renvoie un chemin de capacité PATH_MAX alloué sur le tas
 * avec data contenant une copie de chemin_init
 * ou NULL en cas d'échec
 */
chemin * init(char * chemin_init){
    chemin * res = malloc(sizeof (chemin));
    if(res==NULL) {perror("malloc"); goto error;}
    res->capacity = PATH_MAX;
    unsigned int taille = strlen(chemin_init);
    res->data = malloc(res->capacity);
    if(res->data == NULL){perror("malloc"); goto error;}
    memmove(res->data, chemin_init, taille);
    res->data[taille] = '\0';
    res->length=taille;
    for(int i=taille-1; i>=0; i--){
        if(res->data[i]=='/') {res->pred = res->data+i; break;}
    }
    return res;
error:
free(res);
return NULL;
}

/**
 * prend une chaine de caractères représantant le nom d'un répertoire 
 * et l'ajoute au chemin
 * si le buffer risque de déborder, un autre est alloué de taille capacity * 2 
 * dir_name ne doit ni commencer ni terminer par /
 * renvoie 1 en cas de succes et 0 en cas d'echec
 */
int add_chemin (chemin * chemin, char * dir_name){
    unsigned int taille_src = strlen(dir_name);
    if(chemin->capacity < chemin->length + taille_src + 2) {
        char * tmp = realloc(chemin->data, chemin->capacity*2);
        if(tmp==NULL){
            return 0;
        }
        chemin->data = tmp;
        chemin->capacity*=2;
    }
    if(chemin->length != 1){
        chemin->data[chemin->length] = '/';
        chemin->pred = chemin->data + chemin->length;
        memcpy(chemin->data+chemin->length+1, dir_name, taille_src+1 );
        chemin->length+=taille_src+1;
    }
    else{
        memcpy(chemin->data+chemin->length, dir_name, taille_src+1 );
        chemin->length+=taille_src;
    }
    return 1;
}

/*
 * enlève le dernier nom de répertoire et le / qui le précède
 * si chemin == / la fonction ne le modifie pas et renvoie 0, sinon elle renvoie 1
*/
int truncate_chemin(chemin * chemin){
    if(chemin->length == 1) return 0;
    if(chemin->pred != chemin->data) {
        *chemin->pred = '\0'; 
        chemin->length = chemin->pred - chemin->data;
    }
    else {
        chemin->length = 1;
        *(chemin->pred+1) = '\0';
    }
    for(int i=chemin->length-1; i>=0; i--){
        if(chemin->data[i]=='/') {
            chemin->pred = chemin->data+i; break;
            }
    }
    return 1;
}
/*
    enleve le premier mot du chemin à partir de la guauche tant que celui n'est pas vide 

*/
char* truncate_chemin_gauche(chemin * chemin){
    if(chemin->length == 0||chemin==NULL) return NULL;
    unsigned int ptr1=0,ptr2=1;
    while(chemin->data[ptr2]!='/' && chemin->data[ptr2]!='\0' )
    {
      
        ptr2++;

    }
    char *tmp=malloc(sizeof(char)*(ptr2-ptr1));
    if(tmp==NULL) {perror("malloc"); return NULL;}
    memcpy(tmp,(chemin->data+ptr1+1),(ptr2-ptr1-1));
    tmp[ptr2-ptr1-1]='\0';
    //chemin->data=chemin->data+(ptr2-ptr1);
    char * data_tmp = malloc(chemin->capacity);
    memmove(data_tmp,chemin->data+(ptr2-ptr1), chemin->length-(ptr2-ptr1) +1);
    free(chemin->data);
    chemin->data = data_tmp;
    chemin->length=chemin->length-(ptr2-ptr1);
    
    return tmp;
}

/*
    Traiter un chemin commençant par "/" en enleveant les .. du chemin
*/
chemin *traitement_chemin(chemin * ch)
{
    chemin * res = init("/");
    unsigned int ptr1=0, ptr2=0;
    char *tmp;
    if(ch==NULL)return NULL;
    if(ch->length>1)ptr2=1;
    else{
        NULL;
    }
    
    while( ptr2 < ch->length+1)
    {
        
        if(ch->data[ptr2]=='/' ||ch->data[ptr2]=='\0' )
        {
            tmp=malloc(sizeof(char)*(ptr2-ptr1));
            if(tmp==NULL) {perror("malloc"); return NULL;}
            memcpy(tmp,(ch->data+ptr1+1),(ptr2-ptr1-1));
            tmp[ptr2-ptr1-1]='\0';
           
            if(strcmp(tmp,"..")!=0)
            {
                add_chemin(res,tmp);
            }
            else{
                truncate_chemin(res);
            }
            free(tmp);
            ptr1=ptr2;
            ptr2++;
        }
        else{
            ptr2++;
        }

    }
    return res;
}

void print_chemin(chemin * c){
    printf("length : %d, data : %s\n", c->length, c->data);
}