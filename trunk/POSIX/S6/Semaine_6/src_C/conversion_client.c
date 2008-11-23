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
  
  /* le pipe est suppose cree par le serveur */
  
  /* Nombre d arguments */
  if (argc != 5){
    fprintf(stderr, "Erreur: Client  nombre d'argument invalid.\n");
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "conversion_client <nom_tube_requete> <nom_tube_reponse> <devise> <montant>\n");
    exit(1);
  }

  printf("apres test nb arg \n \n ");
  
  /* la requete a envoye au serveur */
  req.pid_sender = getpid();
  strcpy(req.currency, argv[4]);
  req.amount = (double)atoi(argv[5]);
  
  
  /* ouverture du tube requete en ecriture */

  if((fd_write=open(argv[1],WR_ONLY)) == -1){
    fprintf(stderr,"Erreur : write\n");
    exit (1);
  }
  
  /* ecriture de la requete dans le tube requete*/

  write(argv[1],req,sizeof(conversion_message));

  close(fd_write);

  /* ouverture du tube reponse en lecture */
  
  if((fd_read=open(argv[2],RD_ONLY)) == -1){
    fprintf(stderr,"Erreur : write\n");
    exit (2);
  }
  
  /* lesture de la reponse dans le tube reponse*/
  
  if ( (n=read(argv[2], buffer, sizeof(conversion_message)*NB_CONVERTERS))==-1){
    fprintf(stderr,"Erreur : read\n");
    exit (1);
  }else{
    display_results(req, buffer);
    
  }
  close(fd_read);
  return 0;
}

