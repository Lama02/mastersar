CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib


all :
	@echo "single_converter | exo2-1 | exo3-1 | clean"

single_converter : $(OBJ_D)/single_converter.o $(OBJ_D)/converters.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/single_converter.o : $(SRC_D)/single_converter.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

$(OBJ_D)/converters.o : $(SRC_D)/converters.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo2-1 : $(OBJ_D)/exo2-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo2-1.o : $(SRC_D)/exo2-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo3-1 : $(OBJ_D)/exo3-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo3-1.o : $(SRC_D)/exo3-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)
.PHONY : clean  all