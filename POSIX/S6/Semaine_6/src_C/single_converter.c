/* exo1-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "converters.h"
#include "string.h"

#define BUFMAX sizeof(conversion_message)*NB_CONVERTERS

int main(int argc, char * argv[]){
  pid_t fils;
  conversion_message req;
  int num_conveters, n;
  results_array tab_res;
  
  char buffer[BUFMAX];
  
  /* Creation du pipe*/
  int tubDesc[2];
  if(pipe(tubDesc) == -1){
    fprintf(stderr,"Erreur : pipe\n");
    exit (1);
  }
  
  
  /* Nombre d arguments */
  if (argc != 3){
    fprintf(stderr, "Erreur: nombre d'argument invalid.\n");
    fprintf(stderr, "Exp: \n");
    fprintf(stderr, "%s CNY \"100.0\"\n", argv[0]);
    
    
    exit(1);
  }
  
  /* la requete a envoye au fils*/
  req.pid_sender = getpid();
  strcpy(req.currency, argv[1]);
  req.amount = (double)atoi(argv[2]);
  
  
  
  /* le pere envoie la requete a un process fils */
  if ( (fils=fork()) == 0 ){
    /* je suis dans le fils */
    /* je dois effectuer la conversion */
    printf("\tP%d> Converting %s %.3f\n", getpid(), req.currency, req.amount);
    for (num_conveters=0 ; num_conveters < NB_CONVERTERS ; num_conveters++){
      /* on lance une requete de conversion pour chaque Devise cible (num_conveters) */
      /* le resultat est stocker dans le tableau results_array definie dans la biblio */
      handle_conversion_request(req, &tab_res[num_conveters], num_conveters);
    }
    
    /* le fils ecrit le resultat dans le pipe */
    if ( (write(tubDesc[1],tab_res,sizeof(conversion_message)*NB_CONVERTERS)) == -1 ){
      fprintf(stderr,"Erreur : write\n");
      exit (1);
    }

    close(tubDesc[1]);
    exit(0);
    
  }
  
  /* le pere attend le resultat en lecture dans le tube */
  /* le resultat sera ecrit par le fils */
  if ( (n=read(tubDesc[0], buffer, sizeof(conversion_message)*NB_CONVERTERS))==-1){
    fprintf(stderr,"Erreur : read\n");
    exit (1);
  }else{
    display_results(req, buffer);
  }
  
  close(tubDesc[0]);

  return 0;
}

