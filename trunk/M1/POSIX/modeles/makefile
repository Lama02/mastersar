CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib





exo1-1 : $(OBJ_D)/exo1-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo1-1.o : $(SRC_D)/exo1-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1 : $(OBJ_D)/exo2-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1.o : $(SRC_D)/exo2-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-2 : $(OBJ_D)/exo2-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-2.o : $(SRC_D)/exo2-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 


exo3-1 : $(OBJ_D)/exo3-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo3-1.o : $(SRC_D)/exo3-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo3-2 : $(OBJ_D)/exo3-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo3-2.o : $(SRC_D)/exo3-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 



clean :
	@rm -fr $(SRC_D)/*~ $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D) *~

run:
	$(BIN_D)/$(EXEC)

.PHONY : clean all