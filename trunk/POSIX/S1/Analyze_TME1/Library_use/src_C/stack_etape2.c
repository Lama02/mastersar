/* Fichier: src_C/stack_etape2.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stacks_of_Int'.
   Cf. include/stack.h
*/

/* test: nouvel usage de la pile */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{ /* test: stack_list quand la pile est vide */
  stack_new();
  stack_list();

  push (5); push (-1);  push (0); push (2);
  stack_list();
  stack_new();
  stack_list();

  exit(0);
}
