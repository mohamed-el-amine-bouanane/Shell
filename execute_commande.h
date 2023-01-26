#ifndef EXECUTE_COMMANDE_H
#define EXECUTE_COMMANDE_H

int execute_commande(char ** mots, unsigned int nb_mots, int ret, int *sig);
int contient_redirection (char ** mots, unsigned int nb_mots);
int est_redirection (char *mot);
int indice_redirection (char **mots ,unsigned int nb_mots);
void affiche_tab_mot (char** tab_mot, unsigned int nb_mot);
void free_tab_mot (char** tab_mot);
void pretraitement_redirection (char **mots ,unsigned int nb_mots, int cpt,char **arggauche,char **argdroit );
int redirection_stdin(char** argg, char** argd,int nb_mots,int ret,int *sig);
int redirection_stdout_sans_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig);
int redirection_stdout_avec_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig);
int redirection_stdout_concatenation(char** argg, char** argd,int nb_mots, int ret, int *sig);
int redirection_stderr_concatenation(char** argg, char** argd,int nb_mots, int ret, int *sig);
int redirection_stderr_sans_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig);
int redirection_stderr_avec_ecrasement(char** argg, char** argd,int nb_mots, int ret, int *sig);
int execcmd(char ** arg,int *sig);




#endif