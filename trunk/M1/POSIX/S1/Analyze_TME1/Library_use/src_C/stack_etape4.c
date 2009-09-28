/* Fichier: src_C/stack_etape4.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stacks_of_Int'.
   Cf. include/stack.h
*/

/* test overflow */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

extern int SP;

int main(int argc, char* argv[])
{ int aux;

  stack_new();
  for (aux = 1; aux <= SIZE ; aux++) push (aux);
  
  push(777);
  stack_list();
  if (SP == -1) exit (111);

  exit(0);
}
