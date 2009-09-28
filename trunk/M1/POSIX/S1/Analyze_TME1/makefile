# Fichier: makefile	pour Utilisation de la bibliotheque livree et installee dans ${LIB_INSTALL}

# Cf. les variables exportables (PROJET, DIRECTORIES, LIB_INSTALL, LIB_USE, CRs) du Shell script analyze

CC=/usr/bin/gcc -Wall -ansi

INCLUDE=${LIB_INSTALL}/include
LIB=${LIB_INSTALL}/lib
SRC=${LIB_USE}/src_C
BIN=${LIB_USE}/bin

compilations:
	@echo -n "Compilations des tests: "
	@${CC} -o ${BIN}/stack_etape0 ${SRC}/stack_etape0.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo -n ". "
	@${CC} -o ${BIN}/stack_etape1 ${SRC}/stack_etape1.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo -n ". "
	@${CC} -o ${BIN}/stack_etape2 ${SRC}/stack_etape2.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo -n ". "
	@${CC} -o ${BIN}/stack_etape3 ${SRC}/stack_etape3.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo -n ". "
	@${CC} -o ${BIN}/stack_etape4 ${SRC}/stack_etape4.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo -n ". "
	@${CC} -o ${BIN}/stack_etape5 ${SRC}/stack_etape5.c -I ${INCLUDE} -L ${LIB} -lstack
	@echo "."

clean:
	rm -f tmp/* Les_comptes_rendus/*
	rm -rf Les_repertoires_traites/*
