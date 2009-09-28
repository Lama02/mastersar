/* TD1-Q8.c */

/* Auteurs: Rachid ALAHYANE */
/* 05.10.2008 02:13:41 */ 

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf buff, bufg;

void f() {
  int n = 0; 
  
  while (1) {
    printf("Execute f: %d\n", n++);
    sleep(1);
    //  if (setjmp(buff) == 0)
    // longjmp (bufg, 1);
    commut();
  }
}


void g(){
  int n=0;
  
  while (1){
    printf("Execute g: %d\n", n++);
    sleep(1);
    //if (setjmp(bufg) == 0)
    // longjmp(buff, 1);
    commut();
  }
}

void commut(){
  // TODO
}

void main(){
  if (setjmp(bufg) == 0)
    f();
  else
    g();
}
