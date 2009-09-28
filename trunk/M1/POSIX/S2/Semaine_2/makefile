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

exo1-2 : $(OBJ_D)/exo1-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo1-2.o : $(SRC_D)/exo1-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1-a : $(OBJ_D)/exo2-1-a.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1-a.o : $(SRC_D)/exo2-1-a.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1-b : $(OBJ_D)/exo2-1-b.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1-b.o : $(SRC_D)/exo2-1-b.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1-c : $(OBJ_D)/exo2-1-c.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1-c.o : $(SRC_D)/exo2-1-c.c
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
	@rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)

run:
	$(BIN_D)/$(EXEC)

.PHONY : clean all