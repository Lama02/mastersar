BIN =  convol_par
SRC =  convol_par.c

CC = mpicc
RUN = mpirun
RUN_FLAGS = --hostfile $${MPI_HOSTFILE_PATH}
NB_PROCESS = -c 8
CFLAGS = -lm

all: $(BIN)

 : %.c
	$(CC) $(CFLAGS) -o $(BIN) $<

run: $(BIN)
	$(RUN) $(RUN_FLAGS) $(NB_PROCESS) $< femme10.ras 2  100

clean:
	rm -f $(BIN) *~ post-convolution_filtre*

.PHONY: all run clean 