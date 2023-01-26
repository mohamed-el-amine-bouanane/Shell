#ifndef EXECUTE_LIGNE_H
#define EXECUTE_LIGNE_H

/*Structure utilise pour la manipulation des commandes*/
struct cmd
{
    char ** mots; //Les mots composant la commande
    unsigned int nb_mots; // le nombre de mots de la commande
}typedef cmd;

unsigned int get_nb_mots(char * ligne);
char ** parss_ligne(char * ligne, unsigned int nb_mots);
unsigned int get_nb_cmds(char * ligne);
int get_taille(char** mots_apres_expansion);
cmd *parss_pipeline(char *ligne, unsigned int nb_cmds );
void free_tab_str(char **tab_cmds_str,int nb_cmds);
void free_tab_cmd( cmd * tab_cmds, unsigned int nb_cmds);
void affichage_cmds(cmd * tab_cmds,unsigned int nb_cmds);
int execute_ligne(cmd * cmds, unsigned int nb_cmds, int ret, int *sig);
int fermer_tout_tubes_sauf_un(int **tab_tubes,int nb_tubes, int t, int sp);
int fermer_tout_tubes_sauf_deux(int **tab_tubes,int nb_tubes, int t1, int sp1, int t2, int sp2);




#endif