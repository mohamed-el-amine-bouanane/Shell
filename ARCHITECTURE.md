## Architecture logicielle

Le programme `slash` est constitué de 10 fichiers en langage C et d'un fichier `Makefile` pour la compilation. L'exécution de la commande `make` à la racine du dépôt permet de créer l'exécutable `slash` dans ce même répertoire. Pour lancer l'exécutable, il suffit d'utiliser la commande `./slash`. La commande `make clean` permet de supprimer tous les fichiers compilés.

Parmi les 10 fichiers en C, 
`slash.c` initialise les variables et lance la boucle principale du programme.
`arguments.c` contient des fonctions pour traiter l'expansion des jokers dans les arguments entrés par l'utilisateur. 

Les fichiers `chemin.c` et `chemins.c` implémentent des structures et leurs fonctions associées permettant de manipuler deux types de chemins utilisés dans les algorithmes de `slash`.

Les autres fichiers utilisés par `slash` pour répondre au sujet sont : 
- `etoile.c` et `double_etoile.c` pour l'utilisation de l'étoile et de la double étoile.
- `cd.c` pour exécuter la commande cd.
- `pwd.c` pour exécuter la commande pwd.
- `execute_ligne.c` pour traiter les redirections d'un pipeline.
- `execute_commande.c` pour traiter les autres redirections et l'exécution des commandes internes et externes.

## Structures de données

Les structures de données que le programme utilise se trouvent dans les fichier `chemin.h` `chemins.h` `execute_ligne.h` sous cette représentation :  

- `struct chemins` représente un tabelau de chaînes de caractères tab, il sert à représenter les résultats trouvés dans le traitement d'un chemin contenant  `*` ou `**`, ainsi que la capacité maximale `capacite` et la taille effective `length`.

- `struct chemin` représente une chaîne de caractères qui désigne un chemin relatif ou absolu de notre système de gestion de fichier (champ `data`), ainsi qu'un champ `pred` qui permet de tronquer le chemin, en plus de deux champs `length` qui indique la longueur de notre chemin et `capacite` qui indique la longeur maximal pouvant être stocker dans le tableau sans extension de la capacité.  

- `struct cmd` est une structure utilisée pour représenter les commandes après le traitement d'éventuels pipes.

## Algorithmes implémentés

#### Exécution d'une ligne de commandes
Après la récuperation de la ligne de commandes par la fonction `readline`, on procède comme suit :

1. On effectue un parssing grâce à la fonction `parss_pipeline` qui nous donne un tableau  de commandes representé par un tableau de stucture cmd, qui découpe la ligne selon la chîne de  charactères ` | ` puis pour chaque élément obtenu, on fait un deuxieme parssing via la fonction `parss_ligne` de `execute_ligne.c` qui renvoie un tableau de chaine de caractères, celui-ci subira un dernier parssing à l'aide de la fonction `traiter_arguments` qui substitue les arguments contenant etoile `*` ou `**` par les résultats correspondant à l'expression régulière traitée.                                                                                                                                                                                                      
2. On exécute la fonction `execute_ligne` de `execute_ligne.c` qui prend un tableau de commandes. Si le tableau contient une seule commande on execute la fonction `execute_commande` du fichier `execute_command.c`. Sinon, on procède d'abord au changement des entrées et/ou sorties standards de chaque commande par des tubes anonymes dans le processus qui exécutera finalement cette dernière en utilisant la fonction `execute_commande`.                                                                                                                                
#### Exécution d'une commande
L'exécution de la commande s'effectue dans la fonction `execute_commande` présente dans le fichier `execute_commande.c` où les distinctions suivantes sont traités:           
- Présence d'une redirection, dans ce cas, on fait la redirection. Puis on exécute d'une façon recursive la commande demandée pour pouvoir traiter les autres redirections éventuelles.
- Exécution des commandes internes `pwd`, `cd`, `exit`.    
- Exécution des commandes externes.

#### Expansion des jokers

Pour chaque argument de la commande (nom de la commande compris), la fonction `traiter_arguments`
appelle `expansion_double_etoile` si l'argument commence par `**/`. Sinon `expansion_etoile` si au moins un nom de base dans le chemin commence par `*`. Sinon, ajoute l'argument tel qu'il est dans le tableau de chemins, qu'on concaténera à la fin pour former un seul tableau de chaînes de caractères qui va contenir les résultats des expansions. 

- `expansion_double_etoile` initialise les variables puis appelle la fonction récursive `references_relatives`. cette dernière parcourt l'arborescence et appelle la fonction `expansion_etoile_dir_cur` qui renvoie le résultat de l'expansion de l'étoile à partir du répertoire courant, ou renvoie le chemin sans changement s'il est valide et ne contient pas d'étoile.
- `expansion_etoile` initialise les variables puis appelle la fonction récursive `explorer_arborescence` qui parcours l'arborescence en suivant les liens symboliques et s'appelle récursivement sur les chemins obtenus en substituant les noms de bases qui commencent par `*` par le résultat de son expansion. 