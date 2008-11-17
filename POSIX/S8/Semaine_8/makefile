CC = gcc
CFLAGS = -Wall -ansi -pedantic -O 
LINK = -lrt -lpthread 
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
EXEC = prog



exo1-1 : $(OBJ_D)/capitalize.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ $(LINK)-o $(BIN_D)/$@

$(OBJ_D)/capitalize.o : $(SRC_D)/capitalize.c 
	$(CC) -o $@ $(CFLAGS) -c $<

clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)
.PHONY : clean all