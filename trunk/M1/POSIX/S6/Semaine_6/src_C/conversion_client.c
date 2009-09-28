/* exo1-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "converters.h"
#include "string.h"

#define BUFMAX sizeof(results_array) 


int main(int argc, char * argv[]){
  
  conversion_message req;
  int n;
  int fd_write, fd_read;
  results_array buffer;
  
  /* le pipe est suppose cree par le serveur */
  /* Nombre d arguments */
  if (argc != 5){
    fprintf(stderr, "Erreur: Client  nombre d'argument invalid.\n");
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "conversion_client <nom_tube_requete> <nom_tube_reponse> <devise> <montant>\n");
    exit(1);
  }

  
  /* la requete a envoye au serveur */
  req.pid_sender = getpid();
  strcpy(req.currency, argv[3]);
  req.amount = (double)atoi(argv[4]);

    
  /* ouverture du tube requete en ecriture */
  if((fd_write=open(argv[1],O_WRONLY)) == -1){
    fprintf(stderr,"Erreur : open\n");
    exit(1);
  }
  
  /* ouverture du tube reponse en lecture */
  if((fd_read=open(argv[2],O_RDONLY)) == -1){
    fprintf(stderr,"Erreur : open\n");
    exit (2);
  }
  
  /* ecriture de la requete dans le tube requete*/
  write(fd_write,&req,sizeof(conversion_message));
    
  /* lecture de la reponse dans le tube reponse*/
  if ((n=read(fd_read,buffer,BUFMAX))==-1){
    
    fprintf(stderr,"Erreur : read\n");
    exit (1);
    
  }else{
    
    display_results(req, buffer);
  }
  
  close(fd_write);
  close(fd_read);
  
  return 0;
}

