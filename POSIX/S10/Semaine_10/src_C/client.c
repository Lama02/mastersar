/* client.c */
#include "chat.h"




/* socket de communicaton */
int sock; 
/* threads d'envoie et de reception */
pthread_t thread_reception, thread_envoie;

char pseudo [MAX_PSEUDO];

/* afficher la reponse du serveur */
void print_reponse(chat_request * message){
  printf("\n> %s : %s\n", message->pseudo, message->msg);
  return;
}




void welcome (chat_request * message){
  printf("Hi %s. Bienvenue dans le chat\n", message->pseudo);
}




/* retourne 1 lorsque le message reponse vient du  */
/* serveur et correspond a un QUIT_ACK, 0 sinon    */
 int est_quit_ack(chat_request * reponse_quit){
  return ((strcmp(reponse_quit->msg, "QUIT_ACK") == 0 ) && 
	  (strcmp (reponse_quit->pseudo, "SYSTEM") == 0 ) );
}




void kill_client(int code_erreur){
  /* Fermer la connexion */  
  pthread_cancel(thread_envoie);
  pthread_cancel(thread_reception);
  shutdown(sock,2); 
  close(sock); 
  exit(code_erreur);
}

/* arret du client mais avant il faut  */
/* envoyer un message QUIT au serveur */
/* et attendre un ack */
void stop_client(){
  chat_request req_quit;
  chat_request reponse_quit;
 
  fprintf(stderr, "Stopping client...");
  
  /* formuler le message */
  /* si on estla cela veut dire    */
  /* que le pseudo est deja dans   */
  /* le champ pseudo de la requete */
  strcpy(req_quit.msg, "QUIT");
  strcpy(req_quit.pseudo, pseudo);
  
  /* envoyer le msg au serveur */
  if (write(sock, &req_quit, sizeof(req_quit)) == -1) { 
    perror("write 00"); 
    kill_client(1);
  }
  
  do{
    /* attendre l'acquittement du serveur */
    if (read(sock, &reponse_quit, sizeof(reponse_quit)) == -1) { 
      perror("read 00");
      /* Fermer la connexion */  
      kill_client(1); 
    }
    fprintf(stderr, "DEBUG message recu du serveur %s\n", reponse_quit.msg);
  } while( !est_quit_ack(&reponse_quit)); 
  

  /* arreter les threads d'envoie et de reception */
  /* et rermer la connexion */  
  fprintf(stderr, "OK\n");
  kill_client(1);  
}


/* A la reception de n'importe quel signal  */
/* et avant de tuer le client, la routine   */
/* stop_client est lancee, ceci permet      */
/* d'arreter proprement le client           */
void set_sigaction(){

  int i;

  /* pour le deroutement */
  sigset_t sig_set;
  struct sigaction action;
  
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
}


/************************************/
/* envoyer le message msg au seveur */
void envoyer_message(chat_request msg){
  fprintf(stderr, "DEBUG envoyer_message: %s\n", msg.msg);
  if (write(sock, (chat_request *)&msg, 
	    sizeof(chat_request)) == -1) { 
    perror("write 0"); 
    exit(1); 
  } 
}


/*********************************************************/
/* envoyer le message saisi par l'utilisateur au serveur */
void * envoie_message(){
  chat_request msg;
  strcpy(msg.pseudo, pseudo);
  while(1){
    printf("%s : ", pseudo); 
    if (fgets(msg.msg, MAX_MSG, stdin) == NULL){
      perror("fgets");
      stop_client();
    }
    fprintf(stderr,"DEBUG envoie_message: %s\n", msg.msg);
    envoyer_message(msg);
  }
}

 
/*********************************************/
/* retourne le message envoye par le serveur */
void msg_recu (chat_request * msg_recu){
  if (read(sock, msg_recu, sizeof(chat_request)) == -1) { 
    perror("read 0"); 
    kill_client(1); 
  }
  fprintf(stderr,"DEBUG msg_recu: %s\n", msg_recu->msg);

}


/*********************************************/
/* afficher le message envoye par le serveur */
void * reception_message(){
  chat_request req_recu;
  while(1){
    msg_recu(&req_recu);
    fprintf(stderr,"DEBUG reception_message: %s\n", req_recu.msg);
    /* le message est un QUIT_ACK */
    if (est_quit_ack(&req_recu)){
      kill_client(1); 
    }
    print_reponse(&req_recu);
  }
}

 

/*********************************************/
/* entre du programme                        */
int main(int argc, char *argv[]) 
{ 
  struct sockaddr_in dest; /* Nom du serveur */ 
  struct hostent *hp; 
  
  /* la requete a envoyer au server */
  chat_request req; 
  /* la reponse du serveur */
  chat_request reponse;
  
  
  if (argc != 3) { 
    fprintf(stderr, "Usage : %s machine pseudo\n", argv[0]); 
    exit(1); 
  } 
  
  /* mettre en place le deroutement definie */
  /* precedement a la reception des signaux */
  set_sigaction();
  
  fprintf(stderr, "[INFO] Connecting to the server...");
  /* le pseudo du client */
  strcpy(pseudo, argv[2]);
  
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
  
  fprintf(stderr, "OK connexion\n");
  
  /* construire le message a envoyer */
  strcpy(req.msg, "JOIN");
  strcpy(req.pseudo, pseudo);
  
  /* joindre le serveur       */
  /* envoyer la commande JOIN */
  fprintf(stderr, "[INFO] joining server...");
  envoyer_message(req);
  /* attendre la reception d'un acquittement */
  msg_recu(&reponse);
  
    
  if ( (strcmp(reponse.pseudo, "SYSTEM") != 0 ) ||
       (strcmp(reponse.msg, "JOIN_ACK") != 0)){
    /* si l'aquitement n'est pas bon */
    /* on arrete le client */
    fprintf(stderr,"Error\n");
    stop_client(); 
  }

  fprintf(stderr, "OK\n");
  
  /* le client a bien rejoint le chat, nous allons creer deux threads  */
  /* une pour envoyer les messages au serveur et l'autre pour recevoir */
  /* les messages du serveur */

  welcome(&req);

  if  (pthread_create ((pthread_t *)&thread_reception, NULL, reception_message, (void *)&reponse) != 0) {
    fprintf(stderr,"pthread_create a rencontre un probleme\n");
    stop_client();
    exit(5);
  }
  
  
  if  (pthread_create ((pthread_t *)&thread_envoie, NULL, envoie_message, (void *) &req) != 0) {
    fprintf(stderr,"pthread_create a rencontre un probleme\n");
    stop_client();
    exit(5);
  }
  
  if (pthread_join (thread_reception,NULL) !=0) {
    printf ("pthread_join"); exit (1);
  }
  
  if (pthread_join (thread_envoie,NULL) !=0) {
    printf ("pthread_join"); exit (1);
  }
  
  return(0); 
}
  


