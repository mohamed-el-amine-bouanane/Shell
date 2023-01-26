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

#include "chemin.h"
#include "chemins.h"
#include "etoile.h"

DIR * dir = NULL;

/**
 * prend une chaine de caractères représentant un chemin contenat éventuellement des
 * etoile en prefixe ; et retourne un tableau de chemins qui représente l expansion
 * de la double étoile
 */
chemins* expansion_etoile (char * c){
    chemins* cs=NULL; chemin* chemin=NULL;
    cs = init_chemins();
    if(cs == NULL) goto error;
    int relatif = 0;
    if(*c=='/'){
        c++;
        chemin= init("/");
        if(chemin==NULL) goto error;
        dir = opendir("/");
        if(dir==NULL) goto error;
    }
    else{
        chemin= init("");
        relatif=1;
        if(chemin == NULL) goto error;
        dir = opendir(".");
        if(dir==NULL) goto error;
    }
    chemins* res = explorer_arborescence(c, cs, chemin);
    if (res==NULL) goto error;
    if(relatif){
        for(int i=0; i<cs->length; i++){
            char * tmp =  malloc(strlen(cs->tab[i]));
            if(tmp==NULL) goto error;
            memmove(tmp, cs->tab[i]+1, strlen(cs->tab[i]));
            free(cs->tab[i]);
            cs->tab[i]=tmp;
        }
    }
    if(dir!=NULL) {closedir(dir), dir=NULL;}
    supprime_chemin(chemin);
    chemin=NULL;
    return cs;
    
    error:
    if(dir!=NULL) {closedir(dir); dir=NULL;}
    if(chemin!=NULL) supprime_chemin(chemin);
    if(cs!=NULL) supprime_chemins(cs);
    return NULL;
}

chemins* expansion_etoile_dircur(char * ref, DIR* dircur){
    chemins* cs=NULL; chemin* chemin=NULL;
    cs = init_chemins();
    if(cs == NULL) goto error;
    int relatif = 0;
    
    chemin= init("");
    if(chemin == NULL) goto error;
    int fd_dir = dup(dirfd(dircur));
    dir = fdopendir(fd_dir);
    if(dir==NULL) goto error;
    fd_dir=-1;
    
    chemins* res = explorer_arborescence(ref, cs, chemin);
    if (res==NULL) goto error;
    
    for(int i=0; i<cs->length; i++){
        char * tmp =  malloc(strlen(cs->tab[i]));
        if(tmp==NULL) goto error;
        memmove(tmp, cs->tab[i]+1, strlen(cs->tab[i]));
        free(cs->tab[i]);
        cs->tab[i]=tmp;
    }

    if(dir!=NULL) {closedir(dir), dir=NULL;}
    supprime_chemin(chemin);
    chemin=NULL;
    return cs;
    
    error:
    if(fd_dir>=0) close(fd_dir);
    if(dir!=NULL) {closedir(dir); dir=NULL;}
    if(chemin!=NULL) supprime_chemin(chemin);
    if(cs!=NULL) supprime_chemins(cs);
    return NULL;
}

char * get_name(char * c){
    if (*c == '\0') return NULL;
    char * res;
    char * ptr = c;
    while(1){
        if(*c=='/' || *c=='\0'){
            res=malloc(c-ptr+1);
            if(res==NULL) return NULL;
            memmove(res, ptr, c-ptr);
            res[c-ptr] = '\0';
            break;
        }
        c++;
    }
    return res;
}

chemins * explorer_arborescence (char * c, chemins * cs, chemin * chemin){
    struct dirent * ent=NULL;
    DIR* dir_deux_points = NULL;
    chemins * noms_entrees = NULL;
    DIR * dir_symlink = NULL;
    struct stat st;
    int fd_tmp=-1;
    while(*c=='/') c++;
    char * name = get_name(c);
    rewinddir(dir);
    while(1){
        errno=0;
        ent = readdir(dir);
        if(errno) goto error;
        if(ent == NULL) break;
        if(fstatat(dirfd(dir), ent->d_name, &st, AT_SYMLINK_NOFOLLOW) < 0 ) goto error;
        if(strcmp(ent->d_name, name) == 0 && (*(c + strlen(name))=='\0' || (*(c+strlen(name))=='/' && *(c+strlen(name)+1)=='\0' && S_ISDIR(st.st_mode)) )){
            add_chemin(chemin,name);
            chemins* res = add_chemins(cs,chemin);
            if(res == NULL) goto error;
            truncate_chemin(chemin);
            free(name);
            return res;
        }
        else if(strcmp(ent->d_name, name) == 0){
            if(strcmp(name,".")!=0 && strcmp(name, "..")!=0){
                fd_tmp = openat(dirfd(dir), name, O_RDONLY|O_DIRECTORY);
                if(fd_tmp<0 && errno==ENOTDIR){errno=0; return cs;}
                else if(fd_tmp<0) goto error;
                if(!S_ISLNK(st.st_mode)){
                    closedir(dir);
                    dir=NULL;
                    dir=fdopendir(fd_tmp);
                    fd_tmp = -1;
                }else{
                    dir_symlink = dir;
                    dir=NULL;
                    dir = fdopendir(fd_tmp);
                    fd_tmp=-1;
                }
            }
            else if(strcmp(name,"..")==0){
                fd_tmp = openat(dirfd(dir), name, O_RDONLY|O_DIRECTORY);
                if(fd_tmp<0 && errno==ENOTDIR){errno=0; return cs;}
                else if(fd_tmp<0) goto error;
                dir_deux_points = dir;
                dir=NULL;
                dir=fdopendir(fd_tmp);
                fd_tmp=-1;
            }
            add_chemin(chemin,name);
            chemins * res = explorer_arborescence(c+strlen(name)+1, cs, chemin);
            if(res==NULL) goto error;
            cs=res;

            truncate_chemin(chemin);
            if(strcmp(name,".")!=0 && strcmp(name,"..")!=0){
                if(S_ISLNK(st.st_mode)){
                    closedir(dir);
                    dir = dir_symlink;
                    dir_symlink=NULL;
                }
                else{
                    fd_tmp = openat(dirfd(dir), "..", O_RDONLY|O_DIRECTORY);
                    closedir(dir);
                    dir=NULL;
                    dir=fdopendir(fd_tmp);
                    fd_tmp = -1;
                }
            }
            else if(strcmp(name, "..")==0){
                closedir(dir);
                dir = dir_deux_points;
                dir_deux_points = NULL;
            }
            free(name);
            return cs;
        }
    }
    
    if(*name== '*'){    
        noms_entrees = traitement_etoile (dir, name+1);
        int fd_tmp = -1;    
        for(int j=0; j<noms_entrees->length; j++){
            char * ref = remplace_par_nom(c,name, noms_entrees->tab[j]);
            if(ref==NULL) goto error;
            chemins * res = explorer_arborescence(ref, cs, chemin);
            free(ref); ref=NULL;
            if(res==NULL) goto error;
            cs=res;
        }
        supprime_chemins(noms_entrees);
        noms_entrees = NULL;
    }
    free(name);
    name=NULL;
    return cs;

    error:
    if(fd_tmp != -1) close(fd_tmp);
    if(noms_entrees) supprime_chemins(noms_entrees);
    if(dir) {closedir(dir); dir=NULL;}
    if(dir_deux_points) closedir(dir_deux_points);
    if(name) free(name);
    if(dir_symlink) closedir(dir_symlink);
    return NULL;
}

char* remplace_par_nom(char * c, char* name_toile, char* name){
    int size= strlen(c)+strlen(name)-strlen(name_toile);
    char* res=malloc(size+1);
    if(res==NULL) goto error;
    res[size]='\0';
    memmove(res, name, strlen(name));
    memmove(res+strlen(name), c+strlen(name_toile), strlen(c)-strlen(name_toile));
    return res;
    error:
    return NULL;
}



chemins *traitement_etoile(DIR* dir, char *s)
{
    struct dirent *entry;
    chemin *ch=NULL;
    chemins *chs=init_chemins();

    if (chs==NULL) goto error;
    rewinddir(dir);
    while((entry=readdir(dir))!=NULL){
        if(entry->d_name[0]=='.') continue;
        if(match_suffixe (entry->d_name,s)==0){
            ch=init(entry->d_name);
            if (ch==NULL) goto error;
            chs=add_chemins(chs,ch);
            if(chs==NULL) goto error;
            supprime_chemin(ch);
            ch=NULL;
        }
    }
    return chs;
    
    error:
    if(ch) supprime_chemin(ch);
    if(chs) supprime_chemins(chs);
    return NULL;
}

int match_suffixe (char *dir_name,char *suffixe )
{
    if(strcmp(suffixe,"")==0) return 0;
    if (strlen(suffixe)>strlen(dir_name)) return 1;

    char *tmp= malloc(sizeof(char)*strlen(suffixe)+1);
    if(tmp==NULL) return 1;
    memmove(tmp,dir_name+(strlen(dir_name)-strlen(suffixe)),sizeof(char)*strlen(suffixe));
    tmp[strlen(suffixe)]='\0';
    if(strcmp(tmp,suffixe)==0) {free(tmp);return 0;}
    free(tmp);
    return 1;
}
