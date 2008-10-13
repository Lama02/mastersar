/* Fichier: src_C/stack_etape3.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stacks_of_Int'.
   Cf. include/stack.h
*/

/* test: SIZE numbers empiles */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{ double un_entier;
  double fibnmoins2, fibnmoins1;
  int aux;

  stack_new();
  push(1);
  push(1);
  for (aux = 2; aux <= SIZE - 1; aux++)
    { fibnmoins1 = pop(); fibnmoins2 = pop();
      push (fibnmoins2); push (fibnmoins1); push (fibnmoins1 + fibnmoins2);
    }
  stack_list();

  for (aux = 1; aux <= SIZE; aux++)
    { un_entier = pop();
    }
  stack_list();

  exit(0);
}
