all: $(BIN)
BIN=matmul

mandel: mandel/mandel.c
	gcc -fopenmp $< -o $@

convol: convol/convol.c
	gcc -fopenmp $< -o $@

matmul: matmul/matmul.c
	gcc -fopenmp $< -o $@

clean : 
	rm -rf $(BIN) convol/*~ mandel/*~ matmul/*~ *~

.PHONY: clean all
