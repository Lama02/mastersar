/* Fichier: include/stack.h
   Specifications de gestion simple d'une pile d'entiers,
     de taille definie ci-dessous,
     avec controles de debordements de types overflow (pile pleine) ou underflow (pile vide).
*/

#define SIZE 70 /* size for stack */

#define TOP "[ ----- TOP\n"
#define BASE "  ----- BASE ]\n"

void stack_new (void); /* initialize use of the stack */

void push (double); /* an integer to push, or exit (33); if overflow */

double pop (void); /* return: the number on top, or exit(55); if underflow */

void stack_list (void); /* list numbers pushed from the one on top to the one on base */
