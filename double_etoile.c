#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "double_etoile.h"
#include "chemin.h"
#include "etoile.h"
#include "chemins.h"

#define CAP 10

DIR * dir_cur = NULL;
/**
 * prend une chaine de caractères représentant un chemin de la forme 
 * double_etoile/x/y/... ; et retourne un tableau de chemins qui représente l expansion
 * de la double étoile
 */
chemins* expansion_double_etoile (char * ref){
    ref+=3;
    while(*ref == '/') ref++;
    chemins* cs=NULL; chemin* c=NULL;
    cs = init_chemins();
    if(cs == NULL) goto error;
    c= init("");
    if(c == NULL) goto error;
    dir_cur = opendir(".");
    if(dir_cur==NULL) goto error;
    chemins* res = references_relatives(ref, cs, c);
    if(res==NULL) goto error;
    for(int i=0; i<cs->length; i++){//enlever le '/' au début de chaque chemin
        char * tmp =  malloc(strlen(cs->tab[i]));
        if(tmp==NULL) goto error;
        memmove(tmp, cs->tab[i]+1, strlen(cs->tab[i]));
        free(cs->tab[i]);
        cs->tab[i]=tmp;
    }

    supprime_chemin(c);
    c=NULL;
    return cs;
    
    error:
    if(c!=NULL) supprime_chemin(c);
    if(cs!=NULL) supprime_chemins(cs);
    return NULL;
}

/*
  prend un répertoire et son nombre de sous répertoires
  renvoie un tableau de pointeur vers struct string
  qui représente les noms des sous répertoires de dir
*/
char ** getSousDirs(DIR* dir, int nbSousDir){
  char** sousDirs=malloc(nbSousDir*sizeof(char *));
  int i=0;
  struct dirent * ent=NULL;
  struct stat st;
  rewinddir(dir);
  while(1){
    errno=0;
    ent = readdir(dir);
    if(errno) return NULL;
    if(ent == NULL) break;
  
    //stocker les noms des sous répertoires
    if(fstatat(dirfd(dir),ent->d_name, &st, AT_SYMLINK_NOFOLLOW)<0) {errno= 0;continue;}
    if((S_IXUSR & st.st_mode) == 0 || (S_IRUSR & st.st_mode) == 0) continue;
    if(S_ISDIR(st.st_mode) && *ent->d_name !='.'){
      sousDirs[i]=malloc(strlen(ent->d_name)+1);
      memmove(sousDirs[i], ent->d_name, strlen(ent->d_name)+1);
      i++;
    }
  } 
  return sousDirs;
}


chemins * references_relatives (char * ref, chemins * cs, chemin * c){
    struct dirent * ent=NULL;
    struct stat st;
    int nbSousDir=0;
    char** sousDirs =NULL;
    int fd_tmp = -1;
    rewinddir(dir_cur);
    while(1){//compter le nombre de sous repertoires et ajouter les chemins
        errno=0;
        ent = readdir(dir_cur);
        if(errno) goto error;
        if(ent == NULL) break;
        if(fstatat(dirfd(dir_cur),ent->d_name, &st,AT_SYMLINK_NOFOLLOW)<0) {
            errno=0;
            continue;
        }
        if((S_IXUSR & st.st_mode) == 0 || (S_IRUSR & st.st_mode) == 0) continue;
        if(S_ISDIR(st.st_mode)&& *(ent->d_name)!='.'){ nbSousDir++; }
    }
    if(*ref != '\0'){
        chemins* exp = expansion_etoile_dircur(ref, dir_cur);
        for(int i=0; i<exp->length; i++){
            chemin * tmp = init(c->data);
            add_chemin(tmp, exp->tab[i]);
            add_chemins(cs, tmp);
            supprime_chemin(tmp);
        }
        supprime_chemins(exp);
    }
    
    if(nbSousDir==0){
        return cs;
    }
    sousDirs = getSousDirs(dir_cur, nbSousDir);
    for(int j=0; j<nbSousDir; j++){
        if(*sousDirs[j]!='.'){
            if(add_chemin(c, sousDirs[j])==0) goto error;
            fd_tmp = openat(dirfd(dir_cur), sousDirs[j], O_RDONLY|O_DIRECTORY);
            if(fd_tmp<0) goto error;
            closedir(dir_cur);
            dir_cur=fdopendir(fd_tmp);
            if(dir_cur==NULL) goto error;
            fd_tmp=-1;

            /*ajouter aux chemins*/
            if(*ref == '\0'){//on ajoute le chemin dans le résultat
                add_chemin(c, "");//pour ajouter / à la fin
                add_chemins(cs, c);
                truncate_chemin(c);
            }
            
            chemins* res = references_relatives(ref, cs, c);
            if(res==NULL) goto error;
            truncate_chemin(c);
            fd_tmp = openat(dirfd(dir_cur), "..", O_RDONLY|O_DIRECTORY);
            if(fd_tmp<0) goto error;
            closedir(dir_cur);
            dir_cur=fdopendir(fd_tmp);
            if(dir_cur==NULL) goto error;
            fd_tmp=-1;
        }
    }
    for(int i=0; i<nbSousDir; i++){
        free(sousDirs[i]);
    }
    free(sousDirs);
    return cs;

    error:
    if (dir_cur) closedir(dir_cur);
    if(fd_tmp>=0) close(fd_tmp);
    if(sousDirs) {
        for(int i=0; i<nbSousDir; i++){
            free(sousDirs[i]);
        }
        free(sousDirs);
    }
    return NULL;
}