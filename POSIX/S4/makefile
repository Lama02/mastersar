# Trois lignes a modifier:
Nom1 = ait-oumghar
Nom2 = alahyane
NumeroDeSemaine = 4


MODELES = ../modeles
RACINE = Semaine_${NumeroDeSemaine}

what:
	@echo "create-env voir tarczf tartzf menage clean what analyse"

# pour preparer le menage dans l'arbre avant de le donner a tar:
voir:
	ls -aRl ${RACINE} | more
	@echo "!!! Faire le menage eventuellement !!!"

# faire le menage dans cet arbre (avec rm) pour tous les fichiers de noms generiques:
#	*~, #*#, .*, bin/*, obj/*, lib/*, *.tgz (sauf si explicitement necessaire)
menage: 
	-@find $(RACINE) -name '.*' -exec rm {} \;
	@find $(RACINE) -name '*~' -exec rm {} \;
	@find $(RACINE) -name '#*#' -exec rm {} \;
	@find $(RACINE) -name '*.tgz' -exec rm {} \;
	@(cd $(RACINE) && $(MAKE) clean)
	@echo "-- Le menage est fait !"

create-env:
	@echo -n "-- Creation de votre environnement..."
	@mkdir -p $(RACINE)/include
	@mkdir -p $(RACINE)/lib
	@mkdir -p $(RACINE)/src_C
	@mkdir -p $(RACINE)/obj
	@touch $(RACINE)/README
	@cp $(MODELES)/makefile $(RACINE)/makefile
	@echo "OK";

# pour construire le fichier compresse' apres menage:
tarczf: menage clean
	-@(cp -r $(RACINE) $(RACINE).tmp && find $(RACINE).tmp -name '*.svn' -exec rm -fr {} \; )
	@echo "la copie est faite"
	@(cd ${RACINE}.tmp ; tar czf ../${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz .)
	@(rm -fr $(RACINE).tmp)
	@( test -f ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz && echo "-- Le tar ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz est cree.")
	@rm -fr $(RACINE).tmp

# pour verifier:
tartzf:
	@(tar tzf ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz)

analyse: tarczf
	@(cd Analyze_TME${NumeroDeSemaine} && make clean)
	@(echo "-- Les anciens fichiers d'analyse sont supprimes")
	@(cp ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz Analyze_TME${NumeroDeSemaine}/Les_fichiers_livres)
	@(echo "-- La livraison du tar ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz est faite.")
	@(echo "=== Debut d'analyse ===")
	@(cd Analyze_TME${NumeroDeSemaine} && ./analyze)
clean:
	@rm -f ${Nom1}_${Nom2}.${NumeroDeSemaine}.tgz
