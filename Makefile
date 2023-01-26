
slash : slash.o cd.o pwd.o chemin.o chemins.o etoile.o double_etoile.o arguments.o execute_commande.o execute_ligne.o
	gcc slash.o cd.o pwd.o chemin.o chemins.o etoile.o execute_commande.o execute_ligne.o double_etoile.o arguments.o -L/usr/include -lreadline  -o slash

slash.o : slash.c pwd.h cd.h chemin.h chemins.h etoile.h double_etoile.h arguments.h execute_ligne.h
	gcc -c slash.c -o slash.o

arguments.o : arguments.c arguments.h chemin.h chemins.h etoile.h double_etoile.h
	gcc -c arguments.c -o arguments.o

double_etoile.o : double_etoile.c chemin.h chemins.h etoile.h double_etoile.h
	gcc -c double_etoile.c -o double_etoile.o

etoile.o : etoile.c chemin.h chemins.h etoile.h
	gcc -c etoile.c -o etoile.o

cd.o : cd.c cd.h pwd.h chemin.h
	gcc -c cd.c -o cd.o

pwd.o : pwd.c pwd.h chemin.h
	gcc -c pwd.c -o pwd.o

chemins.o : chemins.c chemins.h chemin.h
	gcc -c chemins.c -o chemins.o

chemin.o : chemin.c chemin.h
	gcc -c chemin.c -o chemin.o
	
execute_commande.o : execute_commande.c execute_commande.h chemin.h cd.h pwd.h
	gcc -c execute_commande.c -o execute_commande.o
execute_ligne.o : execute_ligne.c execute_ligne.h execute_commande.h  arguments.h
	gcc -c  execute_ligne.c -o execute_ligne.o
clean :
	rm slash *.o
