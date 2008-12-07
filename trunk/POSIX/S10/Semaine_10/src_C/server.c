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
#define TAILLE_FILE 10

/* nombre maximum de connectes */
#define MAX_CONNECTES 30

/* port d'ecoute */
#define PORTSERV 9900

/* taille d'un pseudo */
#define MAX_PSEUDO 24

/* taille d'un message */
#define MAX_MSG 1000




/* la struct des messages echanges */
typedef struct req {
  char pseudo [MAX_PSEUDO];
  char msg[MAX_MSG];
} chat_request;

/* la structure d'un connecte */
struct un_connecte {
  /* cette entree est valide ou pas */
  short valid; 
  /* socket de communication attache a ce connecte */
  socket_t scom; 
  /* le pseudo de ce connecte peut servir si on veut */
  /* la liste des connectes */
  char pseudo [MAX_PSEUDO]; 
}

/* liste des connectes */
struct un_connecte tab_connectes[MAX_CONNECTES];

/* nombre de connecte */
int nb_connectes = 0;





/* declaration de la fonction de traitement*/

void * traitement_req(void * scom){
  
  /* rendre la thread detachee */
  /* la thread liberera elle meme ses ressources a la */
  /* fin de son execution */
  pthread_attr_setdetachstate(PTHREAD_CREATE_DETACHED);
  
  /* la requete envoyee par le  client */
  chat_request req_from_cli;
  
  /* le reponse a envoyer au client  */
  chat_request resp_to_cli;
  
  /* pour parcourir la liste des connectes */
  int i;

  /* attendre que le client nous envoie une reqete */
  if (read((int) scom, &req_from_cli, sizeof(req_from_cli))){ 
    perror("read");
    exit(1);
  }

  /*-- traitement du message envoye par le client --*/
  /* le message correspond a JOIN */
  if ( (strcmp(req_from_cli.msg;"JOIN")) == 0 ) {
    
    /* verifier si le nombre maxi de connectes n'est pas atteint */
    /* poser un verro */
    if (nb_connectes >= MAX_CONNECTES) {
      /* pas la peine de trouver une place dans la liste des
	 connecte. bye */
      /* enlever le verro */
      resp_to_cli.pseudo = "SYSTEM";
      resp_to_cli.msg    = "JOIN_MAX_CONNECTIONS";
      write((int) scom, &resp_to_cli, sizeof(resp_to_cli));
      
      /* deconnexion : arreter les envoies/receptions */
      shutdown(scom, 2);
      close(scom);
      pthread_exit((void *)1);      
    }
    
    /* le verro n'est pas encore ete enleve */
    /* incrementer le nombre des connectes */
    nb_connectes++;
    /* enlever le verro */
    
    /* avertir tout le monde que le chatteur pseudo */
    /* a rejoint le chat */
    resp_to_cli.pseudo = "SYSTEM";
    sprintf(resp_to_cli.msg,"%s a rejoint le chat",req_from_cli.pseudo);
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes.[i].valid == 0){
	write ((int) tab_connectes.[i].scom, &resp_to_cli, sizeof(resp_to_cli));
      }
    }

    /* ajouter le client dans la liste des participants */
    /* on a deja reserve notre place messieurs :) */
    /* a la recherche d'une place dans la liste des connectes */
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes.[i].valid == 0){
	tab_connectes.[i].scom = scom;
	tab_connectes.[i].pseudo = req_from_cli.pseudo;
      }
    }
    
    /* envoyer un acquittement pour la connexion au client */
    resp_to_cli.pseudo = "SYSTEM";
    resp_to_cli.msg    = "JOIN_ACK";
    write((int) scom, &resp_to_cli, sizeof(resp_to_cli));
  }
  
  
  /* la thread s'occupera de toutes le requetes de notre client */
  while (1){
    /* le message correspond a QUIT */
    if ( (strcmp(req_from_cli.msg;"QUIT")) == 0 ) {
      
      /* supprimer le client de la liste des participants */
      for (i=0; i< MAX_CONNECTES; i++){
	if (tab_connectes.[i].scom == scom){
	  tab_connectes.[i].valid = 0;
	  break;
	}
      }
      
      /* poser un verrou */
      nb_connectes--;
      /* enlever le verrou */


      /* avertir tout le monde que le chatteur pseudo */
      /* a quitte le chat */
      resp_to_cli.pseudo = "SYSTEM";
      sprintf(resp_to_cli.msg,"%s a quitte le chat",req_from_cli.pseudo);

      /* ici on envoie un acquittement pour la deconnexion au client */
      resp_to_cli.pseudo = "SYSTEM";
      resp_to_cli.msg    = "QUIT_ACK";
      write((int) scom, &resp_to_cli, sizeof(resp_to_cli));
      
      /* deconnexion : arreter les envoies/receptions */
      shutdown(scom, 2);
      close(scom);
      pthread_exit((void *)0);
    
    }else{
      
      
      /* message normal a diffuser a tous les autres clients */
      /* TODO*/
      
      /* poser un verrou sur la liste des connecte */
      for(i=0; i < MAX_CONNECTES; i++){
	if(tab_connectes[i].valid == 1){
	  write(tab_connectes[i].scom, &req_from_cli, sizeof(req_from_cli));
	}
      }
      /* enlever le verrou pose */
    }
    
    /* attendre que le client nous envoie une reqete */
    if (read((int) scom, &req_from_cli, sizeof(req_from_cli))){ 
      perror("read");
      exit(1);
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

  int tid[MAX_CONNECTES];

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
    
    
    
    if  (pthread_create (&tid[i], NULL, traitement_req, communication_desc) != 0) {
      fprintf(stderr,"pthread_create a rencontre un probleme\n");
      exit(5);
    }
    
    i++;


      return 0;
  }
