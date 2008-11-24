CC = gcc
INCLUDE = ./include
CFLAGS = -Wall -ansi -pedantic -O -I $(INCLUDE)
LINK =  -lpthread
SRC_D = ./src_C
BIN_D = ./bin
OBJ_D = ./obj
LIB_D = ./lib





exo1-1 : $(OBJ_D)/exo1-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@

$(OBJ_D)/exo1-1.o : $(SRC_D)/exo1-1.c
	$(CC) -o $@ $(CFLAGS) -c $< 

exo2-1 : $(OBJ_D)/exo2-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@

$(OBJ_D)/exo2-1.o : $(SRC_D)/exo2-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo3-1 : $(OBJ_D)/exo3-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo3-1.o : $(SRC_D)/exo3-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo4-1 : $(OBJ_D)/exo4-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo4-1.o : $(SRC_D)/exo4-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo4-2 : $(OBJ_D)/exo4-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo4-2.o : $(SRC_D)/exo4-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo5-1 : $(OBJ_D)/exo5-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo5-1.o : $(SRC_D)/exo5-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo6-1 : $(OBJ_D)/exo6-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo6-1.o : $(SRC_D)/exo6-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo6-2 : $(OBJ_D)/exo6-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo6-2.o : $(SRC_D)/exo6-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo7-1 : $(OBJ_D)/exo7-1.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo7-1.o : $(SRC_D)/exo7-1.c
	$(CC) -o $@ $(CFLAGS)  -c $< 

exo7-2 : $(OBJ_D)/exo7-2.o
	mkdir -p $(BIN_D) && $(CC) $(CFLAGS) $(LINK) $^ -o $(BIN_D)/$@
$(OBJ_D)/exo7-2.o : $(SRC_D)/exo7-2.c
	$(CC) -o $@ $(CFLAGS)  -c $< 


clean :
	@rm -fr $(OBJ_D)/*.o $(BIN_D)/$(EXEC) $(LIB_D)/*.a $(BIN_D)

run:
	$(BIN_D)/$(EXEC)

.PHONY : clean all