#ifndef ETOILE_H
#define ETOILE_H


#include <dirent.h>
#include "chemin.h"
#include "chemins.h"

char* remplace_par_nom(char * c, char* name_toile, char* name);
chemins* expansion_etoile (char * c);
chemins* expansion_etoile_dircur(char * ref, DIR* dircur);
char * get_name(char * c);
chemins * explorer_arborescence (char * c, chemins * cs, chemin * chemin);
chemins *traitement_etoile(DIR* dir, char *s);
int match_suffixe (char *dir_name,char *suffixe);
#endif