CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
EXEC = prog


all: $(EXEC)
$(EXEC) : $(OBJ_D)/ficindex_enonce_tests.o $(OBJ_D)/ficindex_enonce.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

$(OBJ_D)/ficindex_enonce.o : $(SRC_D)/ficindex_enonce.c 
	$(CC) -o $@ $(CFLAGS) -c $<

$(OBJ_D)/ficindex_enonce_tests.o : $(SRC_D)/ficindex_enonce_tests.c
	$(CC) $(CFLAGS) -c $< -o $@

clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)

.PHONY : clean all