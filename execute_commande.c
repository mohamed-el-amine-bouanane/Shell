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
#include "execute_commande.h"

#define L 0
#define P 1
#define FALSE 0
#define TRUE 1



int execute_commande(char ** mots, unsigned int nb_mots, int ret, int *sig){
    int contient = contient_redirection(mots, nb_mots);
    if (contient){
        int i=indice_redirection(mots,nb_mots);
        if(i==nb_mots-1){
            write(2, "slash : erreur de syntaxe\n", strlen("slash : erreur de syntaxe\n"));
            return 2;
        }
        char **arggauche = malloc(sizeof(char*)*(i+1));
        char **argdroit = malloc(sizeof(char*)*(nb_mots-i));
        pretraitement_redirection (mots ,nb_mots,i,arggauche,argdroit );
        int res;
        switch (contient){
            case 1:
                res = redirection_stdin(arggauche, argdroit,i,ret,sig);
                break;
            case 2:
                res = redirection_stdout_sans_ecrasement(arggauche,argdroit,i,ret,sig);
                break;
            case 3:
                res = redirection_stdout_avec_ecrasement(arggauche,argdroit,i,ret,sig);
                break;
            case 4:
                res = redirection_stdout_concatenation(arggauche,argdroit,i,ret,sig);
                break;
            case 5:
                res = redirection_stderr_sans_ecrasement(arggauche,argdroit,i,ret,sig);
                break;
            case 6:
                res = redirection_stderr_avec_ecrasement(arggauche,argdroit,i,ret,sig);
                break;
            case 7:
                res = redirection_stderr_concatenation(arggauche,argdroit,i,ret,sig);
                break;
            default:
                write(STDERR_FILENO,"Ne devrait jamais se produire\n", sizeof("Ne devrait jamais se produire\n"));
                return 1;
        }
        free(arggauche);
        free(argdroit);
        return res;
    }
    else{
        if(strcmp(mots[0], "pwd")==0){//exécuter pwd
            if(nb_mots==1 || (nb_mots == 2 && strcmp(mots[1],"-L")==0 )){//pwd ou pwd -L
                    return pwd(L);
            }   
            else if(nb_mots == 2 && strcmp(mots[1], "-P")==0){// pwd -P
                return pwd(P);
            }
            else{// erreur 
                char usage[] = "usage : pwd [-L | -P]\n";
                write(STDERR_FILENO, usage, strlen(usage));
                return 1;
            }
        }
        else if(strcmp(mots[0], "exit")==0 && nb_mots<3 ){//exécuter exit
            char usage[] = "usage : exit [val]\n";
            if(nb_mots==1){// exit
                for(int i=0; i<nb_mots; i++){
                    free(mots[i]);   
                }
                free(mots);
                exit(ret);
            }
            else if(nb_mots==2){// exit val
                char * ptr;//pour stocker l'adresse du premier caractère qui n'est pas un nombre
                int v = strtol(mots[1],&ptr, 10);//mettre l'entier correspondant à val dans v
                if(ptr!=mots[1]+strlen(mots[1])) {//val n'est pas un nombre 
                    write(STDERR_FILENO, usage, strlen(usage));
                    return 1;
                }
                else{//val est bien un nombre
                    for(int i=0; i<nb_mots; i++){
                        free(mots[i]);   
                    }
                    free(mots);
                    exit(v);
                }
            }
            else//usage incorrect de exit
                write(STDERR_FILENO, usage, strlen(usage));
                return 1;
        }
        else if(strcmp(mots[0], "cd")==0){//exécuter cd
            int res = traiter_cd(mots, nb_mots);
            return res;
        
        }
        else{//command externe
            int res = execcmd(mots,sig);
            return res;
        }
    }
     
    error:
    perror("slash");
    return 1;
}


int contient_redirection (char ** mots, unsigned int nb_mots){
    int cpt=nb_mots-1;
    while(cpt>=0)
    {
        if (strcmp(mots[cpt],"<")==0)   return 1;          //Cas >
        else if (strcmp(mots[cpt],">")==0) return 2;       //Cas <
        else if (strcmp(mots[cpt],">|")==0) return 3;      //Cas >|
        else if (strcmp(mots[cpt],">>")==0) return 4;      //Cas >>
        else if (strcmp(mots[cpt],"2>")==0) return 5;      //Cas 2>
        else if (strcmp(mots[cpt],"2>|")==0) return 6;     //Cas 2>|
        else if (strcmp(mots[cpt],"2>>")==0) return 7;     //Cas 2>>
        cpt--;

    }
    return 0;
}
int est_redirection (char *mot){
    if (strcmp(mot,">")==0 || strcmp(mot,"<")==0 || strcmp(mot, ">|")==0 || strcmp(mot,">>")==0 || strcmp(mot,"2>")==0 || strcmp(mot,"2>|")==0 || strcmp(mot,"2>>")==0 )
        return 1;
    else
        return 0;
}

int indice_redirection (char **mots ,unsigned int nb_mots){
    int cpt=nb_mots-1;
    int cond=1;

    while(cond){
        if(est_redirection (mots[cpt])){
            cond=0;
        }
        else{
            cpt--;
        }
    }
    return cpt;   
}

/*
    Free un tableau de mot
*/
void free_tab_mot (char** tab_mot){
    int cpt=0;
    char *p=tab_mot[cpt];

    while(p!=NULL){
        free(p);
        cpt++;
        p=tab_mot[cpt];
    }
    free(tab_mot);
}

void pretraitement_redirection (char **mots ,unsigned int nb_mots, int cpt,char **arggauche,char **argdroit ){
    for (int i=0; i< cpt ;i++){
        arggauche[i]=mots[i];
    }
    arggauche[cpt]=NULL;
    for (int i=cpt+1, j=0; i<nb_mots;i++,j++){
        argdroit[j]= mots[i];
    }
    argdroit[nb_mots-cpt-1]=NULL;
}

/* cas cmd > fic*/
int redirection_stdout_sans_ecrasement(char** argg, char** argd,int nb_mots , int ret,int *sig)
{
    if(argd ==NULL || argg ==NULL )goto error;
    int fd= open (argd[0],O_CREAT|O_EXCL|O_WRONLY,0666);
    if(fd==-1)goto error;
    int sav=dup(STDOUT_FILENO);
    dup2(fd,STDOUT_FILENO);
    close(fd);
    int res=  execute_commande(argg,nb_mots,ret, sig);
    dup2(sav,STDOUT_FILENO);
    return res;

    error:
    return 1;
}
/* cas cmd < fic*/
int redirection_stdin(char** argg, char** argd, int nb_mots, int ret, int *sig){
    int fd_in = -1;
    if( (fd_in = open(argd[0], O_RDONLY)) < 0) goto error;
    int sav = dup (STDIN_FILENO);
    if( dup2 (fd_in, STDIN_FILENO)<0) goto error;
    close(fd_in);
    int res = execute_commande(argg, nb_mots, ret, sig);
    dup2(sav, STDIN_FILENO);
    return res;
    
    error:
    return 1;
}


/* cas cmd >| fic*/
int redirection_stdout_avec_ecrasement(char** argg, char** argd, int nb_mots, int ret, int *sig){
    int fd= open (argd[0],O_CREAT|O_WRONLY|O_TRUNC,0666);
    if(fd==-1)goto error;
    int sav = dup(STDOUT_FILENO);
    if(dup2(fd,STDOUT_FILENO)<0) goto error;
    close(fd);
    int res = execute_commande(argg,nb_mots,ret,sig);
    dup2(sav, STDOUT_FILENO);
    return res;

    error:
    return 1;
}

/* cas cmd >> fic*/
int redirection_stdout_concatenation(char** argg, char** argd,int nb_mots, int ret, int *sig)
{
    if(argd ==NULL || argg ==NULL )goto error;
    int fd= open (argd[0],O_CREAT|O_WRONLY|O_APPEND,0666);
    if(fd==-1)goto error;
    int sav = dup(STDOUT_FILENO);
    if(dup2(fd,STDOUT_FILENO)<0) goto error;
    close(fd);
    int res = execute_commande(argg,nb_mots,ret,sig);
    dup2(sav, STDOUT_FILENO);
    return res;

    error:
    return 1;
}

/* cas cmd 2> fic*/
int redirection_stderr_sans_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig)
{
    if(argd ==NULL || argg ==NULL )goto error;
    int fd= open (argd[0],O_CREAT|O_WRONLY|O_EXCL,0666);
    if(fd==-1)goto error;
    int sav = dup(STDERR_FILENO);
    if(dup2(fd,STDERR_FILENO)<0) goto error;
    close(fd);
    int res = execute_commande(argg,nb_mots,ret,sig);
    dup2(sav, STDERR_FILENO);
    return res;

    error:
    return 1;
}

/* cas cmd 2>| fic*/
int redirection_stderr_avec_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig)
{
    if(argd ==NULL || argg ==NULL )goto error;
    int fd= open (argd[0],O_CREAT|O_WRONLY|O_TRUNC,0666);
    if(fd==-1)goto error;
    int sav = dup(STDERR_FILENO);
    if(dup2(fd,STDERR_FILENO)<0) goto error;
    close(fd);
    int res = execute_commande(argg, nb_mots, ret, sig);
    dup2(sav, STDERR_FILENO);
    return res;

    error:
    return 1;
}

/* cas cmd >> fic*/
int redirection_stderr_concatenation(char** argg, char** argd,int nb_mots, int ret, int *sig)
{
    if(argd ==NULL || argg ==NULL )goto error;
    int fd= open (argd[0],O_CREAT|O_WRONLY|O_APPEND,0666);
    if(fd==-1)goto error;
    int sav = dup(STDERR_FILENO);
    if(dup2(fd,STDERR_FILENO)<0) goto error;
    close(fd);
    int res = execute_commande(argg,nb_mots,ret,sig);
    dup2(sav, STDERR_FILENO);
    return res;

    error:
    return 1;
}

/*Fonction qui execute une commande externe ,renvoie la valeur de retour et met a jours sig */
int execcmd(char ** argg,int *sig){
    int status;
    pid_t r=fork();
    if(r == -1){ 
    	perror("fork");
        goto error;
    }
    else if(r == 0){// fils : execution de la commande 
    	//ne pas ignorer SIGINT et SIGTERM pour les commandes externes
        struct sigaction sa = {0};
        sa.sa_handler=SIG_DFL;
        sigaction(SIGINT,&sa,NULL);
        sigaction(SIGTERM,&sa,NULL);   
        if(execvp(argg[0],argg)<0){
            perror("slash");
            if(errno == ENOENT) exit(127);
            if(errno == EACCES) exit(126);
            exit(1);
        }
    }
    else{//pere wait (s)
        wait(&status);
        int valeur_de_retour = 1;
        if(WIFEXITED(status)){
            valeur_de_retour = WEXITSTATUS(status);
        }
        else if(WIFSIGNALED(status)){
            valeur_de_retour = 255;
            *sig = 1;
        }
        return valeur_de_retour;
    }
    error:
    return 1;
}




