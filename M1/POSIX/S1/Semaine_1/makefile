CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib
EXEC = stack_test


all: $(EXEC)
$(EXEC) : $(OBJ_D)/stack_test.o archive
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $^ -o $(BIN_D)/$@

$(OBJ_D)/stack_test.o : $(SRC_D)/stack_test.c 
	$(CC) -o $@ $(CFLAGS) -c $<

$(OBJ_D)/stack.o : $(SRC_D)/stack.c
	$(CC) $(CFLAGS) -c $< -o $@

archive: $(OBJ_D)/stack.o
	ar cr $(LIB_D)/libstack.a $^
clean :
	rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)
.PHONY : clean archive all