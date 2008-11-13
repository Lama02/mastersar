CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
MY_EXEC = my_prog
EXEC = prog
VRAC = ../../Vrac/POSIX/2008oct/TME5/


what:
	@echo "usage:"
	@echo "make $(EXEC)\t pour compiler le programme avec les tests fournis"
	@echo "make $(MY_EXEC)\t pour compiler le programme avec nos propres tests"
	@echo ""

$(EXEC) : $(OBJ_D)/autotestfic.o $(OBJ_D)/ficindex_enonce.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

$(MY_EXEC) : $(OBJ_D)/ficindex_enonce_tests.o $(OBJ_D)/ficindex_enonce.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

$(OBJ_D)/autotestfic.o:
	cp $(VRAC)/autotestfic.o $@

$(OBJ_D)/ficindex_enonce.o : $(SRC_D)/ficindex_enonce.c 
	$(CC) -o $@ $(CFLAGS) -c $<

$(OBJ_D)/ficindex_enonce_tests.o : $(SRC_D)/ficindex_enonce_tests.c
	$(CC) $(CFLAGS) -c $< -o $@

run: 
	$(BIN_D)/$(EXEC)
clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D) fic1

.PHONY : clean all