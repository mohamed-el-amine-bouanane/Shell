#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <paths.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>

#include "chemin.h"
#include "cd.h"
#include "pwd.h"

#define L 0
#define P 1
#define FALSE 0
#define TRUE 1


/*
 * prend un tableau de chaines de caractères contenant 
 * la commande cd et ses arguments, fait un prétraitement 
 * pour déterminer si c'est un cd logique
 * ou physique et le chemin du répertoire ciblé;
 * puis appelle la fonction cd
*/
int traiter_cd(char ** mots, unsigned int nb_mots){
    char * ref=NULL;
    int lp = -1;
    char usage[]="usage : cd [-L | -P] [ref | -]\n";
    if(nb_mots == 1) {ref=getenv("HOME"); lp=L;}// cd --> cd -L $HOME
    else if(nb_mots == 2){
        if(strcmp(mots[1], "-L")==0){// cd -L --> cd -L $HOME
            lp=L;
            ref=getenv("HOME");
        }
        else if(strcmp(mots[1], "-P")==0){// cd -P --> cd -P $HOME
            lp=P;
            ref=getenv("HOME");
        }
        else{// cd ref --> cd -L ref
            lp=L;
            ref=mots[1];
        }
    }
    else if(nb_mots==3){
        if(strcmp(mots[1], "-L")==0) lp=L;// cd -L ref
        else if(strcmp(mots[1], "-P")==0) lp=P;//cd -P ref
        else goto error;//usage incorrect
        
        ref=mots[2];
    }
    else goto error;//usage incorrect
    
    //exécuter la commande
    return cd(lp, ref);

    error:
    write(STDERR_FILENO, usage, strlen(usage));
    return 1;
}


/*
 * prend un entier et une réference
 * exécute cd_logique si lp == L
 * cd_physique si lp == P;
 * retourne 0 en cas de succès; 1 en cas d'erreur
*/
int cd(int lp, char * ref){
    if(strcmp(ref,"-")==0) {//on va dans l'ancien répertoire courant
        ref=getenv("OLDPWD");
        if(ref==NULL){//OLDPWD n'existe pas encore
            char err[] = "cd: « OLDPWD » non défini\n";
            write(STDERR_FILENO, err,strlen(err));
            return 1;
        }
    }

    if(lp == P) return cd_physique(ref);
    if(lp == L) return cd_logique(ref);
    return 1;
}




/**
 * change le répertoire courant en itérprétant ref de manière physique
 * renvoie 0 en cas de succès; 1 en cas d'échec
 */
int cd_physique(char * ref){
    DIR * dir=NULL;
    int fd_suivant=-1;
    char * nouveau_ref = NULL;
    if(ref[0] == '/'){//on commence par la racine
        ref++;
        dir = opendir("/");
        if(dir==NULL) 
            goto error;
    }
    else{// on commence par le répertoire courant
        dir = opendir(".");
        if(dir==NULL)
            goto error;
    }
    char * dir_name = NULL;
    char * ptr = ref;//pointer le caractère suivant le dernier '/'
    while(1){//tant qu'on a pas fini la lecture de ref
        if(*ptr=='\0') break;//on a fini
        if(*ref == '/' || *ref=='\0'){// on extrait le dernier nom de répertoire lu
            dir_name = malloc(ref-ptr+1);
            if(dir_name==NULL) 
                goto error;
            memmove(dir_name, ptr, ref-ptr);//copier le nom du repertoire dans dir_name
            dir_name[ref-ptr]='\0';
            //ouvrir le répertoire dir_name
            fd_suivant = openat(dirfd(dir), dir_name, O_RDONLY|O_DIRECTORY);
            if(fd_suivant<0)
                goto error;
            
            closedir(dir);
            dir=fdopendir(fd_suivant);
            fd_suivant = -1;// pour ne pas close(fd_suivant) en cas d'erreur (closedir(dir) ferme fd_suivant)
            
            if(dir == NULL)
                goto error;
            
            free(dir_name);
            dir_name = NULL;

            if(*ref == '\0') 
                break;
            else 
                ref++;ptr = ref;
        }
        else
            ref++;
    }

    int ret = fchdir(dirfd(dir));//changer le répertoire courant
    if(ret<0)//erreur
        goto error;
    
    //mettre à jour PWD
    nouveau_ref = my_getcwd();
    if(nouveau_ref == NULL) 
        goto error;
    if(setenv("OLDPWD", getenv("PWD"), 1)<0)
        goto error;
    if(setenv("PWD", nouveau_ref, 1)<0)
        goto error;

    free(nouveau_ref);
    closedir(dir);
    return 0;
    
    error:
    perror("cd_p");
    if(dir_name!=NULL) free(dir_name);
    if(dir!=NULL) closedir(dir);
    if(nouveau_ref!=NULL) free(nouveau_ref);
    if(fd_suivant>=0) close(fd_suivant);
    return 1;
}



/**
 * change le répertoire courant en itérprétant ref de manière logique
 * renvoie 0 en cas de succès; 1 en cas d'échec
 */
int cd_logique(char * ref){
    //pretretement enlever les ..
    
    DIR * dir=NULL;
    int fd_suivant=-1;
    chemin * tmp_PWD=NULL;
    chemin * final_PWD=NULL;
    chemin * sav_PWD=init(getenv("PWD"));
    char *tmp=NULL;

    if(sav_PWD==NULL)
        goto error;
    char *sav_ref=malloc(sizeof(char)*strlen(ref)+1);
    if(sav_ref==NULL) 
        goto error;
    strcpy(sav_ref,ref);
    final_PWD=init("/");
    if(final_PWD==NULL)
        goto error;
    
    dir = opendir("/");
    if(dir==NULL)
        goto error;

    if(ref[0] == '/'){//on commence par la racine
        ref++;
        if(ref[0]!='\0'){
            tmp_PWD=init("");
            if(tmp_PWD==NULL)
                goto error;
       
            if(add_chemin(tmp_PWD,ref)==0)
                goto error;

            chemin * res_PWD=traitement_chemin(tmp_PWD); 
            supprime_chemin(tmp_PWD);
            tmp_PWD = res_PWD; 
        }     
    }
    else{
        // on commence par le répertoire courant
        tmp_PWD=init(getenv("PWD"));
        if(add_chemin(tmp_PWD,ref)==0)
            goto error;

        chemin * res_PWD=traitement_chemin(tmp_PWD);
        supprime_chemin(tmp_PWD);
        tmp_PWD = res_PWD;
    }
    if(tmp_PWD!=NULL)
    {
        tmp=truncate_chemin_gauche(tmp_PWD);
    }
    while(tmp!=NULL){
        fd_suivant = openat(dirfd(dir),tmp, O_RDONLY|O_DIRECTORY);
        if(fd_suivant<0)//cd logique n'a pas de sens  
            goto cd_physique;
        if(add_chemin(final_PWD,tmp)==0)
            goto error;

        closedir(dir);
        dir=fdopendir(fd_suivant);
        fd_suivant=-1;
        free(tmp);
        tmp=NULL;
        if(dir == NULL)//cd logique n'a pas de sens 
            goto cd_physique;
        tmp=truncate_chemin_gauche(tmp_PWD);
    }
    int ret = fchdir(dirfd(dir));//changer le répertoire courant
    if(ret<0)//erreur
        goto error;
    
    //mettre à jour PWD
    if(setenv("OLDPWD", getenv("PWD"), 1)<0)
        goto error;
    if(setenv("PWD", final_PWD->data, 1)<0)
        goto error;
    closedir(dir);
    free(sav_ref);
    supprime_chemin(tmp_PWD);
    supprime_chemin(final_PWD);
    supprime_chemin(sav_PWD);
    return 0;

    cd_physique://code à exécuter si cd logique n'a pas de sens 
    if(tmp!=NULL) free(tmp);
    if(dir!=NULL) closedir(dir);
    if(tmp_PWD!=NULL) supprime_chemin(tmp_PWD);
    if(final_PWD!=NULL) supprime_chemin(final_PWD);
    if(sav_PWD!=NULL) supprime_chemin(sav_PWD);
    int ret_physique= cd_physique(sav_ref);
    free(sav_ref);
    return ret_physique;

    error:
    perror("cd");
    if(tmp!=NULL) free(tmp);
    if(dir!=NULL) closedir(dir);
    if(sav_PWD!=NULL) supprime_chemin(sav_PWD);
    if(tmp_PWD!=NULL) supprime_chemin(tmp_PWD);
    if(final_PWD!=NULL) supprime_chemin(final_PWD);
    if(sav_ref!=NULL) free(sav_ref);
    return 1;
}

