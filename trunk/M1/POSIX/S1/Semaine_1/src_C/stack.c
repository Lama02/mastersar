#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

/* type de donnee utilise pour implementer la pile */
double tab[SIZE];
int SP;

/* initialize use of the stack */
void stack_new (void){
  /*vider la pile*/
  SP=70;
}


/* an integer to push, or exit (33); if overflow */
void push (double elem){
  if (SP > 0 ) {
    SP--;
    tab[SP] = elem;
  }else{
    perror("!!! stack_overflow !!!");
    exit(33);
  }
}

/* return: the number on top, or exit (55); if underflow */
double pop (void){
  /*la pile n est pas vide*/
  if (SP < SIZE ) {
    SP++;
    return tab[SP-1];
  }else{
    perror("!!! stack_underflow !!!");
    exit (55);
  }
}



/* list numbers pushed from the one on top to the one on base */
void stack_list (void){
  int i;
  /* la pile n est pas vide */
  printf(TOP);
  if (SP <70){
    for (i=SP; i<70; i++){
      printf("\t%d: %g\n",i,tab[i]);
    }
  }
  printf(BASE);  
}


