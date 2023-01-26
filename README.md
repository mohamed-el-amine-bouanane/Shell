# slash, small laudable shell

## Description
Slash est un interpréteur de commandes (aka shell) interactif reprenant quelques fonctionnalités 
plus ou moins classiques des shells usuels : outre la possibilité d'exécuter toutes les
commandes externes, slash propose quelques commandes internes,
permettre la redirection des flots standard ainsi que les combinaisons
par tube, adapter le prompt à la situation, et permettre l'expansion des
chemins contenant les jokers `*` et `**`.
Slash ne fait pas la gestion des tâches : tous les processus lancés seront en premier-plan.

## Compilation et lancement

- `make` lancera la compilation pour produire les fichiers `.o`
  et le fichier exécutable `slash`.
- `./slash` pour lancer l'interpréteur de commandes. 
- `make clean` pour supprimer les fichiers `.o` et `slash`.