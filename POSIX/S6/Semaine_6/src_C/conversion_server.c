/*  */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "converters.h"
#include "string.h"

#define BUFMAX sizeof(conversion_message)*NB_CONVERTERS

int main(int argc, char * argv[]){
  conversion_message req;
  int num_conveters, n;
  results_array tab_res;
  int  fd_read, fd_write;
  
  char buffer[BUFMAX];

  /* Nombre d arguments */
  if (argc != 3){
    fprintf(stderr, "Erreur: nombre d'argument invalid.\n");
    exit(1);
  }
  
  /* Creation du premier tube nomme */
  if (mkfifo(argv[1],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }
  
  /* Creation du deuxieme tube nomme */
  if (mkfifo(argv[2],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }
  
  /* ouvrir en lecture */
  if (fd_read = open(argv[1], O_RDONLY) == -1) {
    fprintf(stderr,"Erreur : open\n");
    exit(1);
  }
 
  /* nous allons attendre qu'un client nous envoie une requete */
  while (1){
      
  }
  
  return 0;
}

