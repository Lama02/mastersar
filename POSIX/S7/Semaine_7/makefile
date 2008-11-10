CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
EXEC = prog



exo1-2 : $(OBJ_D)/capitalize_bis.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

exo1-1 : $(OBJ_D)/capitalize.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@


$(OBJ_D)/capitalize.o : $(SRC_D)/capitalize.c 
	$(CC) -o $@ $(CFLAGS) -c $<

$(OBJ_D)/capitalize_bis.o : $(SRC_D)/capitalize_bis.c 
	$(CC) -o $@ $(CFLAGS) -c $<

clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)
.PHONY : clean all