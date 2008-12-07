CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib

server : $(OBJ_D)/server.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/server.o : $(SRC_D)/server.c
	$(CC) -o $@ $(CFLAGS) -c $< 

client : $(OBJ_D)/client.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@
$(OBJ_D)/client.o : $(SRC_D)/client.c
	$(CC) -o $@ $(CFLAGS)  -c $< 


clean :
	@rm -fr $(OBJ_D)/*.o  $(LIB_D)/*.a $(BIN_D)

.PHONY : clean all