#define _POSIX_SOURCE 1

#include "stack.h"
#include <stdio.h>

int main(){
  int i;
  /* initialisation de la pile */
  stack_new();
  for (i=0; i<10; i++){
    push(i);
  }
  stack_list();
  printf("Apres deux appels de pop()\n");
  pop();
  pop();
  stack_list();
  
  for (i=0; i<90; i++){
    push(i);
  }

  
  return 0;  
}

