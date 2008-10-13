/* Fichier: src_C/stack_etape0.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stack_of_Int'.
   Cf. include/stack.h
*/

/* test: stack_new: nom=SP et initialisation: SP=SIZE pour empilement de SIZE-1 a 0 */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

extern int SP;

int main(int argc, char* argv[])
{ stack_new();
  printf ("SP= %d\n", SP);
  exit(0);
}
