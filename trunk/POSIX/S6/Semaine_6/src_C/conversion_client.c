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
  int tube_requete, tube_reponses;
  
  char buffer[BUFMAX];
  
  /* le pipe est suppose cree par le serveur */
  
  /* Nombre d arguments */
  if (argc != 5){
    fprintf(stderr, "Erreur: nombre d'argument invalid.\n");
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "conversion_client <nom_tube_requete> <nom_tube_reponse> <devise> <montant>\n");
    exit(1);
  }
  
  /* la requete a envoye au serveur */
  req.pid_sender = getpid();
  strcpy(req.currency, argv[3]);
  req.amount = (double)atoi(argv[4]);
  
  /* le chemin du tube par lequel transitent les requêtes*/
  tube_requete = argv[1];
  
  
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
  
  return 0;
}

