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

/* taille d'un pseudo */
#define MAX_PSEUDO 24

/* taille d'un message */
#define MAX_MSG 1000

/* taille de la file d'attente */
#define TAILLE_FILE 10

/* nombre maximum de connectes */
#define MAX_CONNECTES 30

/* port d'ecoute */
#define PORTSERV 9901


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
  /* id de la thread associee au connecte */
  int tid;
} un_connecte; 



