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


int main(int argc, char * argv[]){
  /* declarations */
   int connexion_desc ;
   int communication_desc;
   struct sockaddr_in socket_in;
   
   
   

   /**** initialisations ****/
   
   bzero((char *)&socket_in, sizeof(struct sockaddr_in));
   /* @IP local*/
   socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
   /* le port d'ecoute */
   socket_in.sin_port = htons(PORTSERV);
   /* domaine de communication (internet...)*/
   socket_in.sin_family = AF_INET;



  

   /* creation de socket de connexion */
   if (connexion_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) == -1){
     perror("socket");
     exit(1)
   }

   /* nommer la socket de connexion */
   if (bind(connexion_desc, (struct sockaddr *) &socket_in, sizeof(socket_in)) == -1){
     perror("bind");
     exit(1);
   }
  
   
   /* creation de la file d'attente des requetes de connexion */
   if (listen (connexion_desc, TAILLE_FILE) == -1){
     perror("listen");
     exit(1);
   }
  
   
   /* attente des requetes des clients */
   
   while(1){
     
     /* creation de socket de communication */
     /* on dois creer un thread pour chaque client */
     
     if (communication_desc =accept(connexion_desc,) == -1){
       perror("socket");
       exit(1);
     }
     
     
     
     
   }


   
   


   
   return 0;
}
