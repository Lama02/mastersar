
BIN=main
OBJ=main.o alloc.o gc.o

DEBUG=1
CFLAGS=-O3 -g -Wall -Werror -std=gnu99 -D_DEBUG=$(DEBUG)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-pthread

.PHONY: all bootstrap tidy clean distclean
.SECONDARY: 
.SUFFIXES:

all: bootstrap

web:
	make -C ../../../..

bootstrap: $(BIN)

$(BIN): $(OBJ)
	gcc $(LDFLAGS) $(CFLAGS) -o $@ $^

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

.%.d: %.c
	gcc -MM $< | sed -e 's/\([^:]*\):\(.*\)/\1 $@: \2 Makefile/' > $@

tidy:
	rm -f *~ \#*

clean:
	rm -f $(OBJ)

distclean: clean
	rm -f $(BIN)

ifneq ($(MAKECMDGOALS),dist)
ifneq ($(MAKECMDGOALS),web)
ifneq ($(MAKECMDGOALS),tidy)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
include $(patsubst %.o, .%.d, $(OBJ))
endif
endif
endif
endif
endif

