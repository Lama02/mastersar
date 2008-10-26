CC = gcc
CFLAGS = -Wall -ansi -pedantic -O
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib




what:
	@echo "exo1-1 | exo1-2 | exo2-1 | exo2-2-a | exo2-2-b | exo2-4 | exo2-5 | exo2-6 | exo3-1 "
exo1-1 : $(OBJ_D)/exo1-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo1-1.o : $(SRC_D)/exo1-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo1-2 : $(OBJ_D)/exo1-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo1-2.o : $(SRC_D)/exo1-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1 : $(OBJ_D)/exo2-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1.o : $(SRC_D)/exo2-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-2-a : $(OBJ_D)/exo2-2-a.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-2-a.o : $(SRC_D)/exo2-2-a.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-2-b : $(OBJ_D)/exo2-2-b.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-2-b.o : $(SRC_D)/exo2-2-b.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-3 : $(OBJ_D)/exo2-3.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-3.o : $(SRC_D)/exo2-3.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-4 : $(OBJ_D)/exo2-4.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-4.o : $(SRC_D)/exo2-4.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-5 : $(OBJ_D)/exo2-5.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-5.o : $(SRC_D)/exo2-5.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-6 : $(OBJ_D)/exo2-6.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-6.o : $(SRC_D)/exo2-6.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo3-1 : $(OBJ_D)/exo3-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo3-1.o : $(SRC_D)/exo3-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 


clean :
	@rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)


.PHONY : clean all what
