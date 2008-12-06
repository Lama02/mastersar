/* server.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>
#include <arpa/inet.h>



/* taille de la file d'attente */
#define TAILLE_FILE 20

/* port d'ecoute */
#define PORTSERV 9900



/* declaration de la fonction de traitement*/

void * traitement_req(void * arg){

  /* declaration d une variable qui contiendra la requete client*/
  chat_request req;

  if((read((int) arg, &req, sizeof(req)))){ 
    perror("read ");
    exit(0);
  }

  /* ici on teste si le message correspond a JOIN */

  if ( (strcmp(req.msg;"JOIN")) == 0 ) {

    /* rajouter le client dans la liste des participants */
    /* TODO */
  


    write((int)arg     write((int)
			     write((int)

				   (int)(int)(int) }
			     }
	  }

      }


}





int main(int argc, char * argv[]){

  /*--------- declarations ---------*/
  int socket_connexion ;
  int socket_communication;
  /* nom de la socket de connexion */
  struct sockaddr_in socket_in;

  /* le nom de l'adresse du client et sa taille */ 
  struct sockaddr_in src;  
  int fromlen = sizeof (src);  

  /*--------- initialisations ---------*/
  
  /* nettre a zero les octets de socket_in */
  bzero((char *)&socket_in, sizeof(struct sockaddr_in));
  /* @IP local*/
  socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
  /* le port d'ecoute */
  socket_in.sin_port = htons(PORTSERV);
  /* domaine de communication (internet...)*/
  socket_in.sin_family = AF_INET;
  
  

  

  /* creation de socket de connexion */
  if (socket_connexion = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == -1){
    perror("socket");
    exit(1)
      }

  /* nommer la socket de connexion */
  if (bind(socket_connexion, (struct sockaddr *) &socket_in, sizeof(socket_in)) == -1){
    perror("bind");
    exit(2);
  }
  
   
  /* creation de la file d'attente des requetes de connexion */
  if (listen (connexion_desc, TAILLE_FILE) == -1){
    perror("listen");
    exit(3);
  }
  
   
  /* attente des requetes des clients */
   
  while(1){
     
    /* creation de socket de communication */
    /* nous allons creer une thread pour chaque client */

    if (communication_desc = accept(connexion_desc,(struct sockaddr *)&src, &fromlen ) == -1){
      perror("socket");
      exit(4);
    }
    
    /* creation d'une thread qui traite la requete */
    
    je vais reflechir un petit peu--ok 9 min
    
    if  (pthread_create (&(tid[i]), NULL, traitement_req, communication_desc) != 0) {
      fprintf(stderr,"pthread_create a rencontre un probleme\n");
      exit(5);
    }






   
  return 0;
}
