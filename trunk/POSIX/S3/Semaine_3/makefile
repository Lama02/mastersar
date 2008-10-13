CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
EXEC = mon_prog
N_EXO = 2-4

all: $(EXEC)

$(EXEC) : $(OBJ_D)/exo$(N_EXO).o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

$(OBJ_D)/exo$(N_EXO).o : $(SRC_D)/exo$(N_EXO).c 
	$(CC) -o $@ $(CFLAGS)  -c $< 

clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)

run:
	$(BIN_D)/$(EXEC)

.PHONY : clean all