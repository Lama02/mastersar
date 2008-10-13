/* Fichier: src_C/stack_etape5.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stacks_of_Int'.
   Cf. include/stack.h
*/

/* test underflow */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

extern int SP;

int main(int argc, char* argv[])
{ double un_entier;

  stack_new();
  un_entier=pop();
  if (SP == SIZE + 1) exit (222);

  exit(0);
}
