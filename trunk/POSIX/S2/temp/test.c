#define _POSIX_SOURCE 1
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void echange(char * dest, char * src, int taille){
  strncpy(dest, src, taille);
  dest[taille-1]='o';
}

int main(int argc,char *argv[]){
  char *dest ;
  int taille = strlen(argv[1]);
  dest = malloc(taille+1 * sizeof(char)); /* il faut oublier le \0 */
  echange (dest, argv[1], taille); 
  printf("la valeur de argv[1]: %s\n",dest);
  return 0;
}
