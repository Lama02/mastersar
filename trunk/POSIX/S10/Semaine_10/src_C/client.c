/* client.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  
#include <signal.h>

/* taille d'un pseudo */
#define MAX_PSEUDO 24

/* taille d'un message */
#define MAX_MSG 1000


/* port a connecter */
#define PORTSERV 9901

/* la struct des messages echanges */
typedef struct req {
  char pseudo [MAX_PSEUDO];
  char msg[MAX_MSG];
} chat_request;

/* socket de communicaton */
int sock; 

/* la requete a envoyer au server */
chat_request req; 
/* la reponse du serveur */
chat_request reponse;

 

/* afficher la reponse du serveur */
void print_reponse(chat_request message){
  /* messages en provenance du systeme */ 
  if ( strcmp( message.pseudo, "SYSTEM") == 0){ 
    if ( strcmp( message.msg   , "JOIN_ACK"  ) == 0){
      printf("%s : Hi <%s>, soit le bienvenue.\n", 
	     message.pseudo, req.pseudo);
      return;
    }
  }
  
  /* messages en provenance des participants */
  printf("%s : %s\n", message.pseudo, message.msg);
}

/*
  while (1)
    select{

      if ( FD_ISSET ())
	scanf(); write();
      if ( FD_ISSET ())
	read(); printf();}
*/


/* arret du client mais avant il faut  */
/* envoyer un message QUIT au serveur */
/* et attendre un ack */
void stop_client(){
  
  fprintf(stderr, "Stopping client...");
  
  /* formuler le messag */
  /* si on estla cela veut dire    */
  /* que le pseudo est deja dans   */
  /* le champ pseudo de la requete */
  strcpy(req.msg, "QUIT");
  
  /* envoyer le msg au serveur */
  if (write(sock, &req, sizeof(req)) == -1) { 
    perror("write 00"); 
    /* Fermer la connexion */  
    shutdown(sock,2); 
    close(sock); 
    exit(2);
  }
  
  /* attendre l'acquittement du serveur */
  if (read(sock, &reponse, sizeof(reponse)) == -1) { 
    perror("read 00"); 
    /* Fermer la connexion */  
    shutdown(sock,2); 
    close(sock); 
    exit(2);
  }
  

  /* on boucle tant que l on a pas recut l acquittement */
  while( (strcmp(reponse.msg, "QUIT_ACK")   != 0 ) || 
	 (strcmp (reponse.pseudo, "SYSTEM") != 0 ) ){ 
    
    if (read(sock, &reponse, sizeof(reponse)) == -1) { 
      perror("read 11"); 
      /* Fermer la connexion */  
      shutdown(sock,2); 
      close(sock); 
      exit(2);
    }
  }
  
  /* Fermer la connexion */  
  shutdown(sock,2); 
  close(sock); 
  fprintf(stderr, "OK\n");
  exit(0); /* tout va bien  */
}




int main(int argc, char *argv[]) 
{ 
  struct sockaddr_in dest; /* Nom du serveur */ 
  struct hostent *hp; 
  
  int i;

  /* pour le deroutement */
  sigset_t sig_set;
  struct sigaction action;
  
  
  
    
  if (argc != 3) { 
    fprintf(stderr, "Usage : %s machine pseudo\n", argv[0]); 
    exit(1); 
  } 
  
  /* le pseudo du client */
  strcpy(req.pseudo, argv[2]);
  
  /* on ne masque aucun signal */
  sigemptyset(&sig_set);
  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = stop_client;
  
  /* a la reception d'un signal on doit */
  /* supprimer toutes les sockets creee */
  /* avant de se suicider ;)*/
  for (i=0; i< _NSIG; i++){
    sigaction(i, &action, NULL);
  }
  
  /* creation de la socket de communication  */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
    perror("socket"); 
    exit(1); 
  } 

  
  /* les infos reseau sur la machine distante */
  /* Remplir la structure dest */ 
  if ((hp = gethostbyname(argv[1])) == NULL) { 
    perror("gethostbyname"); 
    close(sock);
    exit(1); 
  }
   

  /* vider l adresse */ 
  memset((char *)&dest, 0, sizeof(dest));
  /* initialiser les champs de l'adresse destination */
  memcpy((char *)&dest.sin_addr, (char *) hp->h_addr, hp->h_length);  
  dest.sin_family = AF_INET; 
  dest.sin_port = htons(PORTSERV); 
  
  /* Etablir la connexion */ 
  if (connect(sock, (struct sockaddr *) &dest, sizeof(dest)) == -1){ 
    perror("connect");
    close(sock);
    exit(1); 
  } 
  
  /* construire le message a envoyer */
  strcpy(req.msg, "JOIN");

  
  while (1){
    /* Envoyer la requete au serveur */ 
    if (write(sock, &req, sizeof(req)) == -1) { 
      perror("write 0"); 
      exit(1); 
    } 
    
    /* Recevoir la reponse */ 
    if (read(sock, &reponse, sizeof(reponse)) == -1) { 
      perror("read 0"); 
      exit(1); 
    } 
    
    /* afficher la reponse du serveur */
    print_reponse(reponse);
    
    printf("%s : ", req.pseudo); scanf("%s", req.msg);
  }
  
  return(0); 
}
  


