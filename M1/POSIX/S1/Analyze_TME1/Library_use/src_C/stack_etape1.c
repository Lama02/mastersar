/* Fichier: src_C/stack_etape1.c"
   Un module pour tester ou utiliser les fonctions de gestion de la bibliotheque 'Lib_Stacks_of_Int'.
   Cf. include/stack.h
*/

/* test: stack_list quand la pile est vide */
/* test: push et pop */

#define _POSIX_SOURCE 1

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{ stack_new();
  stack_list();

  /* test: push et pop */
  push(29);
  printf ("%g\n",pop ());
  push (12); push (0);
  printf ("%g ", pop ()); printf ("%g\n", pop ());
  push (0); push (9);
  printf ("%g ", pop ()); printf ("%g\n", pop ());
  push (5); push (-1);  push (0); push (2);
  stack_list();
  printf ("%g ", pop ()); printf ("%g ", pop ()); printf ("%g ", pop ()); printf ("%g\n", pop ());
  stack_list();

  exit(0);
}
