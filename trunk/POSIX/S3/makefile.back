# fichier makefile pour livrer vos travaux de semaine en semaine, a installer dans votre repertoire 'POSIX_et_C'
# activation par make (puis 'make voir', puis 'make tarczf', puis 'make tartzf'; ou 'make clean')
# syntaxe des noms de fichiers livres:
# noms en minuscules, et en ordre alphabetique
# usage de '-' pour separer des parties de nom
# pas de prenom sauf en cas d'homonymies qui seraient signalees
#	seul:			Nom1.NumeroDeSemaine.tgz
#	  exemples:		  le-bosseur.1.tgz
#				  toutseul.1.tgz
#	en binome:		Nom1_Nom2.NumeroDeSemaine.tgz
#	  exemples:		  bin_ome.1.tgz
#				  de-la-rue_le-jeune.1.tgz
#	en cas d'homonymie:	Nom1.Prenom1_Nom2.NumeroDeSemaine.tgz
#	  exemples:		  dupont.julien_lautre.1.tgz

# Trois lignes a modifier:
Nom1 = ait-oumghar
Nom2 = alahyane
NumeroDeSemaine = 2

MODELES = ../modeles
RACINE = Semaine_${NumeroDeSemaine}

what:
	@echo "create-env voir tarczf tartzf menage clean what"

# pour preparer le menage dans l'arbre avant de le donner a tar:
voir:
	ls -aRl ${RACINE} | more
	@echo "!!! Faire le menage eventuellement !!!"

# faire le menage dans cet arbre (avec rm) pour tous les fichiers de noms generiques:
#	*~, #*#, .*, bin/*, obj/*, lib/*, *.tgz (sauf si explicitement necessaire)
menage:
	@find $(RACINE) -name '.*' -exec rm {} \;
	@find $(RACINE) -name '*~' -exec rm {} \;
	@find $(RACINE) -name '#*#' -exec rm {} \;
	@find $(RACINE) -name '*.tgz' -exec rm {} \;
	@(cd $(RACINE) && $(MAKE) clean)

create-env:
	@echo -n "Creation de votre environnement..."
	@mkdir -p $(RACINE)/include
	@mkdir -p $(RACINE)/lib
	@mkdir -p $(RACINE)/src_C
	@mkdir -p $(RACINE)/obj
	@touch $(RACINE)/README
	@cp $(MODELES)/makefile $(RACINE)/makefile
	@echo "OK";
# pour construire le fichier compresse' apres menage:
tarczf:
	cd ${RACINE} ; tar czf ../${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz .

# pour verifier:
tartzf:
	tar tzf ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz

clean:
	rm -f ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz
