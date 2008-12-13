/* client.c */
#include "chat.h"
#include "client.h"




/* socket de communication */
int sock; 
/* threads d'envoie et de reception */
pthread_t thread_reception, thread_envoie;

char pseudo [MAX_PSEUDO];





/****************************************************************/
/* afficher la reponse du serveur                               */
/****************************************************************/
void print_reponse(chat_request * message){
  printf("\n> %s : %s\n", message->pseudo, message->msg);
  return;
}




/****************************************************************/
/* Afficher un message d'accueil                                */
/****************************************************************/
void welcome (chat_request * message){
  printf("Hi %s. Bienvenue dans le chat\n", message->pseudo);
}




/****************************************************************/
/* retourne 1 lorsque le message reponse vient du               */
/* serveur et correspond a un QUIT_ACK, 0 sinon                 */
/****************************************************************/
int est_quit_ack(chat_request * reponse_quit){
  /*  fprintf(stderr, "[DEBUG] est_quit_ack: Le message recu est \"%s\".\n", reponse_quit->msg);
      fprintf(stderr, "[DEBUG] est_quit_ack: Le pseudo est \"%s\".\n", reponse_quit->pseudo);*/
  if  ((strcmp(reponse_quit->msg, "QUIT_ACK") == 0 ) && 
       (strcmp (reponse_quit->pseudo, "SYSTEM") == 0 ) ){
    /*    fprintf(stderr,"[DEBUG] est_quit_ack: retourne 1\n"); */
    return 1;
  }else{
    /* fprintf(stderr,"[DEBUG] est_quit_ack: retourne 0\n"); */
    return 0;
  } 
}




/****************************************************************/
/* Arrete les threads d'envoie et de reception */
/****************************************************************/
void cancel_threads_communication(){
  pthread_cancel(thread_envoie);
  pthread_cancel(thread_reception);
}




/****************************************************************/
/* Arreter le client */
/****************************************************************/
void kill_client(int code_erreur){
  /* Fermer la connexion */  
  shutdown(sock,2); 
  close(sock); 
  fprintf(stderr, "Stopping client...OK\n");
  exit(code_erreur);
}




/****************************************************************/
/* arret du client mais avant il faut                           */
/* envoyer un message QUIT au serveur                           */
/* et attendre un ack                                           */
/****************************************************************/
void deconnexion(){
  chat_request req_quit;
  chat_request reponse_quit;
  
    fprintf(stderr, "Stopping client...\n");
  
  cancel_threads_communication();
  
  /* formuler le message */
  /* si on est la cela veut dire    */
  /* que le pseudo est deja dans   */
  /* le champ pseudo de la requete */
  set_requete(&req_quit, pseudo, "QUIT");
  
  /* envoyer le msg au serveur */
  envoyer_message(&req_quit);
  
  do{
    /* attendre l'acquittement du serveur */
    recevoir_reponse(&reponse_quit);
    
    /*    fprintf(stderr, "[DEBUG] deconnexion: Le message recu est \"%s\".\n", reponse_quit.msg);*/
    /* fprintf(stderr, "[DEBUG] deconnexion: Le pseudoest \"%s\".\n", reponse_quit.pseudo);*/
    
    
  } while( !est_quit_ack(&reponse_quit)); 
  
  /*  fprintf(stderr,"[DEBUG] APRES le while \n \n ");*/
  
  /* arreter les threads d'envoie et de reception */
  /* et rermer la connexion */  
  kill_client(1);  
}





/****************************************************************/
/* envoyer le message saisi par l'utilisateur au serveur        */
/****************************************************************/
void * envoie_message(){
  chat_request msg;
  strcpy(msg.pseudo, pseudo);
  while(1){
    printf("<%s> : ", pseudo); 
    if (fgets(msg.msg, MAX_MSG, stdin) == NULL){
      perror("fgets");
      deconnexion();
    }
    envoyer_message(&msg);
  }
}

 



/****************************************************************/
/* afficher le message envoye par le serveur                    */
/* si le message recu est un aquittement de QUIT                */
/* lancer la routine de deconnexion                             */
/****************************************************************/
void * reception_message(){
  chat_request reponse;
  while(1){
    recevoir_reponse(&reponse);
    /*fprintf(stderr,"DEBUG reception_message: %s\n", reponse.msg);*/
    /* le message est un QUIT_ACK */
    if (est_quit_ack(&reponse)){
      /* todo quitter proprement */
      kill_client(1); 
    }
    print_reponse(&reponse);
  }
}





/****************************************************************/
/* retourne le message envoye par le serveur                    */
/****************************************************************/
void recevoir_reponse (chat_request * reponse){
  if (read(sock, reponse, sizeof(chat_request)) == -1) { 
    perror("read 0"); 
    kill_client(1); 
  }
  
  /*  fprintf(stderr,"[DEBUG] recevoir_reponse : %s\n", reponse->msg);*/
}




/****************************************************************/
/* envoyer le message msg au seveur                             */
/****************************************************************/
void envoyer_message(chat_request * message){
  /* fprintf(stderr, "[DEBUG] envoyer_message: %s\n", message->msg);*/
  if (write(sock, (chat_request *)message, 
	    sizeof(chat_request)) == -1) { 
    perror("write 0"); 
    exit(1); 
  } 
  /*  fprintf(stderr, "[DEBUG] envoyer_message: message: %s envoye \n", message->msg);*/
}




/****************************************************************/
/* A la reception de n'importe quel signal                      */
/* et avant de tuer le client, la routine                       */
/* deconnexion est lancee, ceci permet                          */
/* d'arreter proprement le client                               */
/****************************************************************/
void init_deroutement(){

  int i;

  /* pour le deroutement */
  sigset_t sig_set;
  struct sigaction action;
  
  /* on ne masque aucun signal */
  sigemptyset(&sig_set);
  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = deconnexion;
  
  /* a la reception d'un signal on doit */
  /* supprimer toutes les sockets creee */
  /* avant de se suicider ;)*/
  for (i=0; i< _NSIG; i++){
      sigaction(i, &action, NULL);
  }  
}



/****************************************************************/
/* la socket de communication                                   */
/****************************************************************/
void init_socket_communication( char * hostname){
  
  struct sockaddr_in dest; /* Nom du serveur */ 
  struct hostent *hp; 

  /* creation de la socket de communication  */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
    perror("socket"); 
    exit(1); 
  } 
  
  /* les infos reseau sur la machine distante */
  /* Remplir la structure dest */ 
  if ((hp = gethostbyname(hostname)) == NULL) { 
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
}




/****************************************************************/
/* formuler la requete */
/****************************************************************/
void set_requete(chat_request * requete, char * pseudo, char * message){
  strcpy(requete->msg, message);
  strcpy(requete->pseudo, pseudo);
}

 

/****************************************************************/
/* Joindre le serveur */
/****************************************************************/
void join_server(){
  chat_request req, reponse;
  
  /* construire le message a envoyer */
  set_requete(&req, pseudo, "JOIN");
  
  /* joindre le serveur       */
  /* envoyer la commande JOIN */
    fprintf(stderr, "[INFO] joining server...\n");
    /*
      fprintf(stderr,"[DEBUG] j'envoie le message %s\n", req.msg);
    */
  envoyer_message(&req);

  /* attendre la reception d'un acquittement */
  recevoir_reponse(&reponse);

  /* 
     fprintf(stderr, "[DEBUG] apres msg-recu %s  \n" , reponse.msg);    
   */
  if ( (strcmp(reponse.pseudo, "SYSTEM") != 0 ) ||
       (strcmp(reponse.msg, "JOIN_ACK") != 0)){
    /* si l'aquitement n'est pas bon */
    /* on arrete le client */
    fprintf(stderr, "[INFO] joining server...ERROR\n");
    deconnexion(); 
  }
    fprintf(stderr, "[INFO] joining server...OK\n");
  welcome(&req);  
}



/****************************************************************/
/* initialise les threads de reception et d'envoie */
/****************************************************************/
void init_threads_communication(chat_request *requete, chat_request * reponse){
  
  if  (pthread_create ((pthread_t *)&thread_reception, NULL, reception_message, (void *)reponse) != 0) {
    fprintf(stderr,"[ERROR] pthread_create a rencontre un probleme\n");
    deconnexion();
    exit(5);
  }
  
  if  (pthread_create ((pthread_t *)&thread_envoie, NULL, envoie_message, (void *) requete) != 0) {
    fprintf(stderr,"[ERROR] pthread_create a rencontre un probleme\n");
    deconnexion();
    exit(5);
  }
  
  if (pthread_join (thread_reception,NULL) !=0) {
    fprintf (stderr,"[ERROR] pthread_join\n"); 
    exit (1);
  }
  
  if (pthread_join (thread_envoie,NULL) !=0) {
    fprintf (stderr, "[ERROR] pthread_join\n"); exit (1);
  }
}



/****************************************************************/
/* entree du programme                                           */
/****************************************************************/
int main(int argc, char *argv[]) 
{ 
  chat_request requete, reponse;
  if (argc != 3) { 
    fprintf(stderr, "Usage : %s machine pseudo\n", argv[0]); 
    exit(1); 
  } 
  
  /* mettre en place le deroutement  */
  init_deroutement();
  
  fprintf(stderr, "[INFO] Connecting to the server...");
  /* le pseudo du client */
  strcpy(pseudo, argv[2]);
  
  init_socket_communication(argv[1]);
  
  fprintf(stderr, "[INFO] Connecting to the server...OK\n");
  
  /* joindre le serveur */
  join_server();

  /* le client a bien rejoint le chat, nous allons creer deux threads  */
  /* une pour envoyer les messages au serveur et l'autre pour recevoir */
  /* les messages du serveur */  
  init_threads_communication(&requete, &reponse); 
  
  
  return(0); 
}
  


