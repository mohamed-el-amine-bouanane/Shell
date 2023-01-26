#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <paths.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <signal.h>




#include "execute_commande.h"
#include "execute_ligne.h"
#include "arguments.h"

#define L 0
#define P 1
#define FALSE 0
#define TRUE 1



/**
 * prend une ligne et renvoie le nombre d'arguments qu'elle contient (nom de la commande inclu) 
 */
unsigned int get_nb_mots(char * ligne){
    unsigned int count=0;
    int trouve = FALSE;
    unsigned int strlen_ligne = strlen(ligne);
    for(int i=0; i<strlen_ligne; i++){
        if(i==strlen_ligne-1 && ligne[i]!=' ') count ++;
        else if(ligne[i]==' ' && trouve) {count++;trouve=0;}
        else if(ligne[i]!=' ') trouve=1;
        else trouve = 0;
    }
    return count;
}

/*
 * prend une ligne saisie par l'utilisateur et le nombre de mots
 * qu'elle contient 
 * renvoie un tableau conteant les mots de la ligne 
*/
char ** parss_ligne(char * ligne, unsigned int nb_mots){
    unsigned int strlen_ligne = strlen(ligne);
    char ** mots = malloc(nb_mots * sizeof(char*));
    if(mots==NULL) goto error;
    for (int i=0;i<nb_mots;i++){
        mots[i] = NULL;
    }
    unsigned int count=0;
    int i=0, k=0;
    int trouve = FALSE;
    for(int j=0; j<strlen_ligne+1; j++){
        if((j==strlen_ligne || ligne[j]==' ') && trouve){
            mots[i] = malloc(count+1);
            if(mots[i]==NULL) {goto error;}
            memmove(mots[i], ligne+k, count);
            mots[i][count]='\0';
            k=j+1;
            count=0;
            i++;
            trouve=0;
        }
        else if(ligne[j]==' ') {trouve=0; k++;}
        else {count++;trouve=1;}
    }
    return mots;
    error:
    perror("parss_ligne");
    for(int i=0; i<nb_mots; i++){
            if(mots[i]!=NULL) free(mots[i]);   
    }
    if(mots!=NULL) free(mots);
    return NULL;
}

/**
 * retourne le nombre de commandes que contient la ligne
 * séparées par des pipes
 */
unsigned int get_nb_cmds(char* ligne){
    unsigned int count=0;
    int trouve = FALSE;
    unsigned int strlen_ligne = strlen(ligne);
    for(int i=1; i<strlen_ligne-1; i++){
        if(ligne[i]=='|' && ligne[i-1]==' ' && ligne[i+1]==' ') 
            count++;
    }
    count++;
    return count;
}

/**
 * prend un tableau de chaînes de caractères qui se termine par
 * NULL, et retourne sa taille (Sans compter la case NULL)
*/
int get_taille(char** mots_apres_expansion){
    int count=0;

    for(char**  a = mots_apres_expansion; *a!=NULL; a++){
        count++;
    }
    return count;
}

/**
 * prend une ligne de commandes contenant eventuellement des pipes
 * et renvoie un tableau de commandes 
*/
cmd* parss_pipeline(char *ligne, unsigned int nb_cmds ){
    cmd* tab_cmds = malloc(sizeof(cmd)*nb_cmds);
    if(!tab_cmds) goto error;
    
    for(int i=0; i<nb_cmds;i++)
        tab_cmds[i].mots=NULL;

    char** tab_cmds_str = malloc(sizeof(char*)*nb_cmds);
    if(tab_cmds_str == NULL) goto error;

    for(int i=0; i<nb_cmds; i++)
        tab_cmds_str[i]=NULL;

    int p1=0,p2=0;
    int i=0,count=0;

    while(p2<strlen(ligne)+1){
        if((p2>0 && ligne[p2]=='|' && ligne[p2-1]==' ')&&(p2<strlen(ligne)-1 && ligne[p2+1]==' ') || (p2 ==strlen(ligne))) {
            tab_cmds_str[i]=malloc(sizeof(char)*count+1);
            if(tab_cmds_str[i]==NULL) goto error;
            if(tab_cmds_str[i]==NULL) goto error;
            memmove(tab_cmds_str[i], ligne+p1,count);
            tab_cmds_str[i][count]= '\0';
            p1=p2+1;
            p2++;
            count=0;
            i++;  
        }
        else{
            p2++;
            count++;
        }
    }
    int nb_mots_tmp=0;
    for(int i=0; i<nb_cmds;i++){
        nb_mots_tmp=get_nb_mots(tab_cmds_str[i]);
        if(nb_mots_tmp==0)//commande vide entre pipes
            goto error;
        char ** mots=parss_ligne(tab_cmds_str[i],nb_mots_tmp);
        if(mots==NULL) goto error;
        
        tab_cmds[i].mots = traiter_arguments(mots, nb_mots_tmp);
        if(tab_cmds[i].mots==NULL) goto error;
        tab_cmds[i].nb_mots=get_taille(tab_cmds[i].mots);

        free_tab_str(mots,nb_mots_tmp);
    }
    free_tab_str(tab_cmds_str,nb_cmds);
    return tab_cmds;

    error:
    if(tab_cmds_str) free_tab_str(tab_cmds_str,nb_cmds);
    if(tab_cmds) free_tab_cmd(tab_cmds, nb_cmds);
    return NULL;
}

void free_tab_str(char **tab_cmds_str,int nb_cmds){
    for(int i=0; i<nb_cmds;i++){
        if(tab_cmds_str[i])
            free(tab_cmds_str[i]);
    }
    free(tab_cmds_str);
}

void free_tab_cmd( cmd * tab_cmds, unsigned int nb_cmds){
    for (int i=0;i<nb_cmds;i++){
        if(tab_cmds[i].mots)
            free_tab_mot (tab_cmds[i].mots);
    }
    free(tab_cmds);
}


int execute_ligne(cmd * cmds, unsigned int nb_cmds, int ret, int *sig){
    int nb_tubes=nb_cmds-1;
    int** tab_tubes = NULL;
    if(nb_cmds==1){
        if(cmds[0].nb_mots!=0)
            return execute_commande(cmds[0].mots, cmds[0].nb_mots, ret, sig);
    }
    else {
        tab_tubes=malloc(sizeof(int*)*(nb_tubes));
        if(!tab_tubes) goto error;
        for(int i=0; i<nb_tubes; i++)
            tab_tubes[i]=NULL;
        for (int i=0; i<nb_tubes;i++) {
            tab_tubes[i]=malloc(sizeof(int)*2);
            if(!tab_tubes[i]) goto error;
            if(pipe(tab_tubes[i])==-1) {
                free(tab_tubes[i]);
                tab_tubes[i]=NULL;
                goto error;
            }
        }

        /*Traitement du premier pipe */
        int r=fork();
        if(r<0)goto error;
        if(r==0){//Processus fils
            fermer_tout_tubes_sauf_un(tab_tubes,nb_tubes, 0,1);
            if(dup2(tab_tubes[0][1],STDOUT_FILENO)<0) goto error;// Redirection de la sortie standard de cmd0 vers le premeir ecrivain
            int ret;
            ret=execute_commande(cmds[0].mots, cmds[0].nb_mots, ret, sig);
            close(tab_tubes[0][1]);
            exit(ret);
        }
        /*Traitement des autre pipe sauf le dernier*/
        for(int i=0;i<nb_tubes-1;i++){
            r=fork();
            if(r<0)goto error;
            if(r==0){//Processus fils
                fermer_tout_tubes_sauf_deux(tab_tubes,nb_tubes, i,0,i+1,1);
                if(dup2(tab_tubes[i][0],STDIN_FILENO)<0) goto error;
                if(dup2(tab_tubes[i+1][1],STDOUT_FILENO)<0) goto error;
                int ret =execute_commande(cmds[i+1].mots, cmds[i+1].nb_mots, ret, sig);
                close(tab_tubes[i][0]);
                close(tab_tubes[i+1][1]);
                exit(ret);
            }
        }
        /*Traitement du dernier pipe*/
        r=fork();
        if(r<0)goto error;
        if(r==0){//Processus fils
            fermer_tout_tubes_sauf_un(tab_tubes,nb_tubes, (nb_tubes-1),0);
            if(dup2(tab_tubes[nb_tubes-1][0],STDIN_FILENO)<0) goto error;// Redirection de l entre standard de cmd(n) vers le dernier lecteur 
            int ret;
            ret=execute_commande(cmds[nb_cmds-1].mots, cmds[nb_cmds-1].nb_mots, ret, sig);
            close(tab_tubes[nb_tubes-1][0]);
            exit(ret);
        }
        pid_t p ;
        int ret = 0, i=0;
        while(1){
            int wstatus;
            p = wait(&wstatus);
            if(p==-1){
                if(errno == ECHILD)
                    break;
                else
                    goto error;
            }
            if (WIFEXITED(wstatus)){
                int ret_tmp = WEXITSTATUS(wstatus);
                if(ret_tmp>ret)
                    ret=ret_tmp;
            }
            if(i==0)
                close(tab_tubes[0][1]);
            else if(i==nb_cmds-1)
                close(tab_tubes[nb_tubes-1][0]);
            else{
                close(tab_tubes[i-1][0]);
                close(tab_tubes[i][1]);
            }
            i++;
        }
        //fermer_tous_les_tubes(tab_tubes, nb_tubes);                
        for(int i=0; i<nb_tubes;i++){
            free(tab_tubes[i]);
        }
        free(tab_tubes);
        return ret;    
    }
    error:
    if(tab_tubes){
        for(int i=0; i<nb_tubes; i++){
            if(tab_tubes[i]){
                close(tab_tubes[i][0]);
                close(tab_tubes[i][1]);
                free(tab_tubes[i]);
            }
        }
        free(tab_tubes);
    }
    return 1;
}


int fermer_tout_tubes_sauf_un(int **tab_tubes,int nb_tubes, int t, int sp){
    for( int i=0; i<nb_tubes;i++){
        if( i!= t){
            if(close(tab_tubes[i][0])==-1) goto error;
            tab_tubes[i][0]=-1;
            if(close(tab_tubes[i][1])==-1) goto error;
            tab_tubes[i][1]=-1;
        }
        else{
            if(sp==0){
                if(close(tab_tubes[i][1])==-1)goto error;
                tab_tubes[i][1]=-1;
            }
            else {
                if(close(tab_tubes[i][0])==-1)goto error;
                tab_tubes[i][0]=-1;
            }
        }
    }
    return 0;
    error:
    perror("fermer_tout_tubes_sauf_un");
    return 1;
}


int fermer_tout_tubes_sauf_deux(int **tab_tubes,int nb_tubes, int t1, int sp1, int t2, int sp2){
    for(int i=0; i<nb_tubes;i++){
        if( i!= t1 && i!=t2){
            if(close(tab_tubes[i][0])==-1) goto error;
            tab_tubes[i][0]=-1;
            if(close(tab_tubes[i][1])==-1) goto error;
            tab_tubes[i][1]=-1;
        }
        else{
            if(i==t1) {
            if(sp1==0) {
                if(close(tab_tubes[i][1])==-1) goto error;
                tab_tubes[i][1]=-1;
            }
            else {
                if(close(tab_tubes[i][0])==-1)goto error;   
                tab_tubes[i][0]=-1;
            }
            }
            else{
                if(sp2==0) {
                    if(close(tab_tubes[i][1])==-1)goto error;
                    tab_tubes[i][1]=-1;
                }
                else {
                    if(close(tab_tubes[i][0])==-1)goto error;   
                    tab_tubes[i][0]=-1;
                }
            }
        }  
    }
    return 0;
    error:
    perror("fermer_tout_tubes_sauf_deux");
    return 1;
}