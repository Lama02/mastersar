/* server.c */
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
#include <mqueue.h>
#include <sys/stat.h>

/* taille de la file d'attente */
#define TAILLE_FILE 10

/* nombre maximum de connectes */
#define MAX_CONNECTES 30

/* port d'ecoute */
#define PORTSERV 9901

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
typedef struct _un_connecte {
  /* cette entree est valide ou pas */
  short valid; 
  /* socket de communication attache a ce connecte */
  int scom; 
  /* le pseudo de ce connecte peut servir si on veut */
  /* la liste des connectes */
  char pseudo [MAX_PSEUDO]; 
} un_connecte; 

/* liste des connectes */
un_connecte tab_connectes[MAX_CONNECTES];

/* nombre de connecte */
int nb_connectes = 0;

/* la socket de connexion */
int socket_connexion ;

/* identifiant de la file de message */
mqd_t fil;


/* stoper la communication */
void stop_communication(void * scom){
  /* deconnexion : arreter les envoies/receptions */
  shutdown(*(int *)scom, 2);
  close(*(int *)scom);
}


/* lorsque le serveur recoit un signal, il doit se terminer */
/* mais avant cela il doit fermer toutes les sockets de communication */
/* qu'il a crees ainsi que la socket de connexion */
void stop_server(){
  int i;

  fprintf(stderr, "Stopping server...");
  /* fermons d'abord celle de connexion comme ca */
  /* on creera plus de socket de communication */
  close(socket_connexion);
  
  

  /* fermons les sockets de communications */
  /* todo posons un verrou sur la liste */
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].valid == 1){
      stop_communication((void *) tab_connectes[i].scom);
    }
  }
  /* todo enlever le verrou sur la lise */
  
  /* fermer et supprimer la file de messages */
  mq_unlink("/file");
  close(fil);
  
  fprintf(stderr, "OK\n");
  exit(0);
}



/* gestion des logs */
void add_log(char * log){
  fprintf(stderr,"[INFO] %s\n",log);
}



/* declaration de la fonction de traitement*/

void * traitement_req(void * scom){

  int i;

  /* la requete envoyee par le  client */
  chat_request req_from_cli;
  
  /* le reponse a envoyer au client  */
  chat_request resp_to_cli;

  /* attendre que le client nous envoie une reqete */
  if ((read(*(int *) scom, &req_from_cli, sizeof(req_from_cli))) == -1){ 
    perror("read 33");
    stop_communication(scom);
    pthread_exit((void *)1);      
  }
  
  /*-- traitement du message envoye par le client --*/
  /* le message correspond a JOIN */
  if ( (strcmp(req_from_cli.msg, "JOIN")) == 0 ) {
    
    fprintf(stderr, "[INFO] Le client %s veut joindre le serveur\n",req_from_cli.pseudo);
    /* verifier si le nombre maxi de connectes n'est pas atteint */
    /* poser un verro */
    if (nb_connectes >= MAX_CONNECTES) {
      /* pas la peine de trouver une place dans la liste des
	 connecte. bye */
      /* enlever le verro */
      strcpy (resp_to_cli.pseudo, "SYSTEM");
      strcpy (resp_to_cli.msg, "JOIN_MAX_CONNECTIONS");
      write(*(int *)scom, &resp_to_cli, sizeof(resp_to_cli));
      fprintf(stderr, "[INFO] Rejet du client %s. plus de place sur le serveur\n",req_from_cli.pseudo);
      /* deconnexion : arreter les envoies/receptions */
      stop_communication(scom);
      pthread_exit((void *)1);      
    }
    
    /* le verro n'est pas encore ete enleve */
    /* incrementer le nombre des connectes */
    nb_connectes++;
    /* enlever le verro */
    
    /* avertir tout le monde que le chatteur pseudo */
    /* a rejoint le chat */
    /* strcpy(resp_to_cli.pseudo,"SYSTEM");
    sprintf(resp_to_cli.msg,"%s a rejoint le chat",req_from_cli.pseudo);
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes[i].valid == 1){ 
	write (tab_connectes[i].scom, &resp_to_cli, sizeof(resp_to_cli));
      }
      }*/
    
    strcpy(resp_to_cli.pseudo,"SYSTEM");
    sprintf(resp_to_cli.msg,"%s a rejoint le chat",req_from_cli.pseudo);
    /* ajout du message dans la file de message*/
    if(mq_send(fil, (char *) &resp_to_cli , MAX_MSG, 0) == -1){
      perror("mq_send");
      exit(1);
    }
    
    /* ajouter le client dans la liste des participants */
    /* on a deja reserve notre place messieurs :) */
    /* a la recherche d'une place dans la liste des connectes */
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes[i].valid == 0){
	tab_connectes[i].valid = 1;
	tab_connectes[i].scom = *(int *)scom;
	printf("DEBUG i=%d scom=%d\n", i, tab_connectes[i].scom);
	strcpy(tab_connectes[i].pseudo, req_from_cli.pseudo);
	break;
      }
    }
    
    /* envoyer un acquittement pour la connexion au client */
    strcpy(resp_to_cli.pseudo, "SYSTEM");
    strcpy(resp_to_cli.msg, "JOIN_ACK");
    if (write(*(int *) scom, &resp_to_cli, sizeof(resp_to_cli)) == -1){
      perror("write44");
      stop_communication(scom);
      pthread_exit((void *)1);      
    }
    
    add_log("le client peut joindre le serveur");
    
  }else{
    /* le message recu n est pas un join */
    /* ce n'est pas possible a ce niveau */
    fprintf(stderr, "[ERROR] Le client %s ne peut pas joindre le serveur\n", 
	    req_from_cli.pseudo);
    /* deconnexion : arreter les envoies/receptions */
      stop_communication(scom);
      pthread_exit((void *)1);      
  }
  
  
  
  while (1){
    /* scenario normal: soit message normal, soit un quit */
    
    /* attendre que le client nous envoie une requete */
    if (read(*(int *) scom, &req_from_cli, sizeof(req_from_cli)) == -1){  
      perror("read 44");
      stop_communication(scom);
      pthread_exit((void *)1);      
    }
    
    
    /* le message correspond a QUIT */
    if ( (strcmp(req_from_cli.msg, "QUIT")) == 0 ) {

      add_log("le client veut quitter le chat");
      /* supprimer le client de la liste des participants */
      for (i=0; i< MAX_CONNECTES; i++){
	if (tab_connectes[i].scom == *(int *)scom){
	  tab_connectes[i].valid = 0;
	  break;
	}
      }
      
      /* poser un verrou */
      nb_connectes--;
      /* enlever le verrou */


      /* avertir tout le monde que le chatteur pseudo */
      /* a quitte le chat */
      strcpy(resp_to_cli.pseudo, "SYSTEM");
      sprintf(resp_to_cli.msg,"%s a quitte le chat",req_from_cli.pseudo);
      /* mettre le message dans la file */
      if(mq_send(fil, (char *) &resp_to_cli, sizeof(chat_request),0) == -1){
	perror("mq_send");
	exit(1);
      }
      
      
      /* ici on envoie un acquittement pour la deconnexion au client */
      strcpy(resp_to_cli.pseudo, "SYSTEM");
      strcpy(resp_to_cli.msg, "QUIT_ACK");
      write(*(int *) scom, &resp_to_cli, sizeof(resp_to_cli));
      
      add_log("arreter la communication avec le client");
      /* deconnexion : arreter les envoies/receptions */
      stop_communication(scom);
      pthread_exit((void *)0);
      
    }else{
      
      /* Message normal a diffuser a tous les autres clients */
      if(mq_send(fil, (char *) &req_from_cli, sizeof(chat_request),0) == -1){
	perror("mq_send");
	exit(1);
      }
      
    }
  } 
}



int main(int argc, char * argv[]){
  
  /*--------- declarations ---------*/
  int socket_communication;
  
  /* nom de la socket de connexion */
  struct sockaddr_in socket_in;

  /* le nom de l'adresse du client et sa taille */ 
  struct sockaddr_in src;  
  int fromlen = sizeof (src);  

  /* stocker les tid des theads creees */
  int tid[MAX_CONNECTES];
  int i = 0; 

  /* pour le deroutement */
  sigset_t sig_set;
  struct sigaction action;
 

  
  /*--------- initialisations ---------*/
  
  /* nettre a zero les octets de socket_in */
  memset((char *)&socket_in, 0,sizeof(socket_in));
  /* @IP local*/
  socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
  /* le port d'ecoute */
  socket_in.sin_port = htons(PORTSERV);
  /* domaine de communication (internet...)*/
  socket_in.sin_family = AF_INET;
  
  

  /* demarrage du serveur */
  fprintf(stderr,"Starting server...");

  /* on ne masque aucun signal */
  sigemptyset(&sig_set);

  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = stop_server;


  /* a la reception d'un signal on doit */
  /* supprimer toutes les sockets creee */
  /* avant de se suicider ;)*/
  for (i=0; i< _NSIG; i++){
    sigaction(i, &action, NULL);
  }


  /* creation de la file de message */
  
  if( (fil = mq_open("/file",O_RDWR | O_CREAT, 0600, NULL)) == (mqd_t) -1){
    perror("mq_open"); 
    exit(1);
}
  
  /* creation de socket de connexion */
  if ((socket_connexion = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("socket");
    close(fil);
    exit(1);
  }
  
  /* nommer la socket de connexion */
  if ((bind(socket_connexion, (struct sockaddr *) &socket_in, sizeof(socket_in))) == -1){
    perror("bind");
    close(fil);
    exit(2);
  }
  
   
  /* creation de la file d'attente des requetes de connexion */
  if ((listen (socket_connexion, TAILLE_FILE)) == -1){
    perror("listen");
    close (socket_connexion);
    close(fil);
    exit(3);
  }

  /* le serveur est up */ 
  fprintf(stderr, "OK\n");

  /* le transfomer en demon */
  if (fork() != 0) exit (0);

  /* attente des requetes des clients */
  while(1){ 
    /* creation de socket de communication */
    /* nous allons creer une thread pour chaque client */
    
    if ((socket_communication = accept(socket_connexion, (struct sockaddr *)&src, (socklen_t *)&fromlen )) == -1){
      perror("accept");
      close (socket_connexion);
      close(fil);
      exit(4);
    }
    
    /* creation d'une thread qui traite la requete */
    
    if  (pthread_create ((pthread_t *)&tid[i], NULL, traitement_req, (void *)&socket_communication) != 0) {
      fprintf(stderr,"pthread_create a rencontre un probleme\n");
      close (socket_connexion);
      close(fil);
      exit(5);
    }
    
    /* rendre la thread detachee */
    /* la thread liberera elle meme ses ressources a la */
    /* fin de son execution */
    if(pthread_detach(tid[i]) != 0){
      perror("pthread_detach");
      close (socket_connexion);
      close(fil);
      exit(1);
    }
    /* pour stocker le tid de la prochaine thread */
    i++;
  }
  

  return 0;    
}


