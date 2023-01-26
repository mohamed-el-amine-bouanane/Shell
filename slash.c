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

#include "chemin.h"
#include "cd.h"
#include "pwd.h"
#include "etoile.h"
#include "arguments.h"
#include "execute_commande.h"
#include "execute_ligne.h"



#define L 0
#define P 1
#define FALSE 0
#define TRUE 1
#define LENGTH_PROMPT 30
#define LENGTH_COULEUR_PROMPT 28

 
int sig=0;//pour indiquer que le processus fils a été terminé par un signal




char *myPrompt(int ret,char *pwd){
    char retString[4];
    int length_return_value = sprintf(retString, "%d", ret);
    int  LENGTH_PURE_PROMPT = 30-(length_return_value+4);
    int  LENGTH_SUB_PROMPT = LENGTH_PURE_PROMPT-3;
    char *Prompt=malloc((LENGTH_COULEUR_PROMPT+LENGTH_PROMPT+1)*sizeof(char));
    if(Prompt==NULL) {perror("malloc myPrompt");return NULL;}
    if (ret == 0){
        memmove(Prompt, "\001\033[32m\002[0]\001\033[36m\002", 18);
    }
    else if(sig==1) { 
        sig=0;
        memmove(Prompt, "\001\033[91m\002[SIG]\001\033[36m\002", 20);
    }
    else {
        memmove(Prompt, "\001\033[91m\002[", 9);
        memcpy(Prompt+strlen(Prompt),retString,length_return_value+1);
        memmove(Prompt+strlen(Prompt), "]\001\033[36m\002", 9);
    }
    if (strlen(pwd)+strlen(Prompt)<=LENGTH_PROMPT+14) {memmove(Prompt+strlen(Prompt),pwd, strlen(pwd)+1);}
    else{ 
        char substring[LENGTH_SUB_PROMPT+1];
        memcpy(substring, pwd+strlen(pwd)-LENGTH_SUB_PROMPT, LENGTH_SUB_PROMPT+1);     
        memmove(Prompt+strlen(Prompt), "...",4);
        memmove(Prompt+strlen(Prompt), substring, strlen(substring)+1);
    }
    memmove(Prompt+strlen(Prompt), "\001\033[00m\002$ ", 10);
    return Prompt;
}


 
int main(int argc, char ** argv){
    int ret=0;//valeur de retour des commandes 
    struct sigaction sa = {0};
    sa.sa_handler=SIG_IGN;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTERM,&sa,NULL);   
    rl_outstream = stderr;
    while(1){
        char * prompt =myPrompt(ret,getenv("PWD"));
        char * ligne = readline(prompt);
        free(prompt);
        if(ligne==NULL) 
            break;
        unsigned int nb_mots = get_nb_mots(ligne);
        if(nb_mots == 0){free(ligne);continue;}//la ligne est vide
        unsigned int nb_cmds = get_nb_cmds(ligne);
        cmd *tab_cmds=parss_pipeline(ligne,nb_cmds);
        free(ligne);
        if(tab_cmds==NULL){
            ret=2;
            write(2, "slash : erreur de syntaxe\n", strlen("slash : erreur de syntaxe\n"));
            continue;
        }
        if(nb_cmds==0) continue;
        ret = execute_ligne(tab_cmds, nb_cmds, ret, & sig);
        //libérer les ressources utilisées pour stocker les arguments
        free_tab_cmd(tab_cmds,nb_cmds);
    }
    
    return ret;//retourner la valeur de retour de la dernière commande
}
