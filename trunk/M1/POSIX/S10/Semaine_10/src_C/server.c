/* "server.c" */
#include "chat.h"
#include "server.h"
#include <errno.h>


/* liste des connectes */
un_connecte tab_connectes[MAX_CONNECTES];

/* nombre de connecte */
int nb_connectes = 0;

/* la socket de connexion */
int socket_connexion ;

/* thread d'envoie des messages */
pthread_t thread_envoie;

/* mutex pour proteger la liste de messages */
pthread_mutex_t mutex_liste_message = PTHREAD_MUTEX_INITIALIZER; 
/* mutex pour proteger la variable nb_connectes */
pthread_mutex_t mutex_nb_connectes  = PTHREAD_MUTEX_INITIALIZER; 
/* mutex pour proteger la liste des connectes */
pthread_mutex_t mutex_tab_connectes  = PTHREAD_MUTEX_INITIALIZER; 

/* identifiant de la file de message */
mqd_t fil;



/****************************************************************/
/* stopper la communication                                     */
/****************************************************************/
void stop_communication(int scom){
  /* deconnexion : arreter les envoies/receptions */
  shutdown(scom, 2);
  close(scom);
}




/****************************************************************/
/* lorsque le serveur recoit un signal, il doit se terminer     */
/* mais avant cela il doit fermer toutes les sockets de         */
/* communication qu'il a crees ainsi que la socket de connexion */
/****************************************************************/
void stop_server(){
  int i;
  
  chat_request shutdown_msg;
 
  fprintf(stderr, "Stopping server...\n");
  /* fermons d'abord celle de connexion comme ca */
  /* on creera plus de socket de communication */
  close(socket_connexion);
  
  /* avertir tout le monde que le serveur est */
  /* entrain de s'arreter */
  set_requete(&shutdown_msg, "SYSTEM", "SERVER_DOWN");
  /* fermons les sockets de communications */
  /* posons un verrou sur la liste */
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].valid == 1){
      envoyer_reponse(tab_connectes[i].scom, &shutdown_msg);
      stop_communication(tab_connectes[i].scom);
    }
  }
  /* enlever le verrou sur la lise */
  pthread_mutex_unlock(&mutex_tab_connectes);

  fprintf(stderr, "Stopping server...OK\n");
  
  exit(0);
}




/****************************************************************/
/* verifie si la requete est un JOIN                            */
/* retourne 1 si c'est le cas 0 sinon                           */
/****************************************************************/
int est_join(chat_request * req){
  if ( (strcmp(req->msg, "JOIN")) == 0 ) {
    /* c est un join*/
    return 1;
  }else{
    return 0;
  }
}




/****************************************************************/
/* verifie si la requete est un QUIT                            */
/* retourne 1 si c'est le cas 0 sinon                           */
/****************************************************************/
int est_quit(chat_request * req){
  if ( (strncmp(req->msg, "QUIT",4)) == 0 ) {
    /* c est un quit */
    return 1;
  }else{
    return 0;
  }
}




/****************************************************************/
/* retourne le premier index libre de la                        */
/* table des connectes. -1 si plus de place                     */
/* libre                                                        */
/****************************************************************/
int get_free_index(){
  int i;
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].valid == 0){
      tab_connectes[i].valid = 1;
      pthread_mutex_unlock(&mutex_tab_connectes);
      return i;
    }
  }
  pthread_mutex_unlock(&mutex_tab_connectes);
  return -1;
}
 



/****************************************************************/
/* supprime le client de la liste des connectes                 */
/****************************************************************/
void del_connecte (int scom){
  int i;
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].scom == scom){
      tab_connectes[i].valid = 0;
      break;
    }
  }
  pthread_mutex_unlock(&mutex_tab_connectes);
}




/****************************************************************/
/* mettre le message message dans la liste des messages         */
/****************************************************************/
void put_message(int scom, chat_request * message){
  /*
    fprintf(stderr, "[DEBUG] on est dans la fct put_message\n");
  */
  pthread_mutex_lock(&mutex_liste_message);

  if(mq_send(fil, (char *) message , sizeof(chat_request), 1) == -1){
    perror("mq_send");
    stop_communication(scom);
    pthread_mutex_unlock(&mutex_liste_message);
    pthread_exit((void *)1);
  }
  pthread_mutex_unlock(&mutex_liste_message);
}




/****************************************************************/
/* extrait le message de la file des messages                   */
/****************************************************************/
void pop_message( chat_request * message){
  if (mq_receive(fil, (char*) message, 
		 sizeof(chat_request)  , NULL ) == -1){
    perror("mq_receive"); 
    stop_server();
  }
}




/****************************************************************/
/* recevoir une requete du client                              */
/****************************************************************/
void recevoir_requete(int scom, chat_request * requete){
  /* attendre que le client nous envoie une reqete */
  if (read(scom, requete, sizeof(chat_request)) == -1){ 
    perror("read");
    stop_communication(scom);
    pthread_exit((void *)1);      
  }
}




/****************************************************************/
/* envoyer une reponse au client                                */
/****************************************************************/
void envoyer_reponse(int scom, chat_request *reponse){

  if (write(scom, reponse, sizeof(chat_request)) == -1){
    perror("write");
    stop_communication(scom);
    pthread_exit((void *)1);       
  }
}




/****************************************************************/
/* cette fonction diffuse les messages se                       */
/* trouvant dans la file des messages a tous                    */
/* les participants du chat                                     */
/****************************************************************/
void * diffuser_messages(){
  int i;
  /* le message a envoyer */
  chat_request resp_to_cli;
  
  while(1){
    pop_message(&resp_to_cli);
    /* envoyer le message */
    pthread_mutex_lock(&mutex_tab_connectes);
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes[i].valid == 1){ 
	envoyer_reponse(tab_connectes[i].scom, &resp_to_cli);  
      }
    }
    pthread_mutex_unlock(&mutex_tab_connectes);
  }
}




/****************************************************************/
/* suppression de la liste des connectes                        */
/****************************************************************/
void deconnexion(int scom, char * pseudo){
  fprintf(stderr,"[INFO] le client %s quitte le chat\n", pseudo);
  
  /* supprimer le client de la liste des participants */
  del_connecte(scom);
  
  /* poser un verrou */
  pthread_mutex_lock(&mutex_nb_connectes);
  nb_connectes--;
  /* enlever le verrou */
  pthread_mutex_unlock(&mutex_nb_connectes);
}




/****************************************************************/
/* Avertie tous les participants que le client                  */
/* pseudo est en train de quitter le chat                       */
/* Supprimer le client de la liste des connectes                */
/****************************************************************/
void traitement_quit(int scom, char * pseudo){
  
  chat_request resp_to_cli;
  
  /* enlever le client de la liste des connectes */
  deconnexion(scom, pseudo);
  
  /* + avertir tout le monde que le chatteur pseudo  */
  /* a quitte le chat                                */
  /* + Envoie un aquittement au client qui a demande */
  /* la deconnexion                                  */
  strcpy(resp_to_cli.pseudo, "SYSTEM");
  sprintf(resp_to_cli.msg,"%s a quitte le chat", pseudo);
  /*
    fprintf(stderr, "[DEBUG] le message \"%s\" sera mis ds la file\n",
    resp_to_cli.msg);
  */
  /* mettre le message dans la file */
  put_message(scom, &resp_to_cli);
  
  fprintf(stderr, "[INFO] envoyer un QUIT_ACK a %s \n", pseudo);
  
  /* ici on envoie un acquittement pour la deconnexion au client */
  strcpy(resp_to_cli.pseudo, "SYSTEM");
  strcpy(resp_to_cli.msg, "QUIT_ACK");
  envoyer_reponse(scom, &resp_to_cli);
  
  fprintf(stderr, "[INFO] Arreter la communication avec le client %s\n", pseudo);
  /* deconnexion : arreter les envoies/receptions */
  stop_communication(scom);
  fprintf(stderr, "[INFO] fin de communication avec %s. BYE\n", pseudo);
  pthread_exit((void *)0);
}




/****************************************************************/
/* ajoute le client a la liste des connectes                    */
/****************************************************************/
void add_connecte(int scom, char * pseudo){
  int i;
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].scom == scom){
      strcpy(tab_connectes[i].pseudo, pseudo);
      /*
	fprintf(stderr,"[DEBUG] Ajout de %s a la liste des connectes\n", pseudo);
	fprintf(stderr,"[DEBUG] %s \tindice=%d \t scm=%d \n",pseudo, i, scom);
      */
      break;
    }
  }
  fprintf(stderr, "[INFO] le client %s est mis dans la liste des connectes\n",
	  pseudo);
  
  pthread_mutex_unlock(&mutex_tab_connectes);
}




/****************************************************************/
/* traitement a faire dans le cas ou le message                 */
/* recu correspond a un join                                    */
/* + avertir tout le monde que le client a joint                */
/*   le serveur                                                 */
/* + ajouter le client dans la liste des connectes              */
/****************************************************************/
void traitement_join(int scom, char * pseudo){
  
  chat_request resp_to_cli;

  fprintf(stderr, "[INFO] Le client %s a fait un JOIN\n", pseudo);
  
  /* envoyer un acquittement pour la connexion au client */
  strcpy(resp_to_cli.pseudo, "SYSTEM");
  strcpy(resp_to_cli.msg, "JOIN_ACK");
  envoyer_reponse(scom, &resp_to_cli);
  
  /* le message a diffuser */    
  strcpy(resp_to_cli.pseudo,"SYSTEM");
  sprintf(resp_to_cli.msg,"%s a rejoint le chat",pseudo);
  
  /*
    fprintf(stderr, "[DEBUG] le message \"%s\" sera mis ds la file\n",
    resp_to_cli.msg);
  */
  
  /* ajout du message dans la file des messages*/
  put_message(scom, &resp_to_cli);
  
  /* ajouter le client dans la liste des participants */
  /* on a deja reserve notre place messieurs :) */
  add_connecte(scom, pseudo);
  
  fprintf(stderr,"[INFO] Le client %s peut joindre le serveur\n", pseudo);     
}




/****************************************************************/
/* traitement des requetes envoyees par le client               */
/****************************************************************/
void * traitement_req(void * scom){
  
  /* requete envoyee par le client */
  chat_request req_from_cli;
  
  /*
    fprintf(stderr, "[DEBUG] Traitement de la requete avec scom = %d\n",
    *(int *)scom); 
    */
  
  /* attendre que le client nous envoie une requete */
  
  recevoir_requete(*(int *) scom, &req_from_cli);
  
  /*-- traitement du message envoye par le client --*/
  /* le message correspond a JOIN */
  if (est_join(&req_from_cli) == 1){
    traitement_join(*(int *)scom, req_from_cli.pseudo);
  }else{
    /* le message recu n'est pas un join              */
    /* normalement ce n'est pas possible              */
    /* a ce niveau, mais bon on verifie quand meme    */
    fprintf(stderr, "[ERROR] Le client %s ne peut pas joindre le serveur\n", 
	    req_from_cli.pseudo);
    /* deconnexion : arreter les envoies/receptions */
    stop_communication(*(int *)scom);
    pthread_exit((void *)1);      
  }
  
  while (1){
    /* scenario normal: soit message normal, soit un quit */
    
    /* attendre que le client nous envoie une requete */
    recevoir_requete(*(int *) scom, &req_from_cli);
    
    /* le message correspond a QUIT */
    /*
      fprintf("[DEBUG] message lu par le serveur : %s\n", req_from_cli.msg);
    */
    if (est_quit(&req_from_cli) == 1){
      traitement_quit(*(int*)scom, req_from_cli.pseudo);
    }else{  
      /* Message normal a diffuser a tous les autres clients */
      put_message(*(int*)scom, &req_from_cli);
    }
  } 
}




/****************************************************************/
/* met en place le deroutement. Lors de la reception de         */
/* n'importe quel signal le serveur se termine proprement       */
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
  action.sa_handler = stop_server;

  /* a la reception d'un signal on doit */
  /* supprimer toutes les sockets creee */
  /* avant de se suicider ;)*/
  for (i=0; i< _NSIG; i++){
    sigaction(i, &action, NULL);
  }
}




/****************************************************************/
/* mecanisme de diffusion                                       */
/****************************************************************/
void init_file_messages(){
  
  struct mq_attr fil_attrs;
  
  fil_attrs.mq_flags = 0;
  fil_attrs.mq_curmsgs = 0;
  fil_attrs.mq_maxmsg = 10;
  fil_attrs.mq_msgsize = sizeof(chat_request);
  
  /* creation de la file de message */
  if( (fil = mq_open("/file", O_RDWR | O_CREAT | O_EXCL , 0600, 
		     &fil_attrs)) == (mqd_t) -1){
    if (errno == EEXIST) {
      mq_unlink("/file");
      if( (fil = mq_open("/file", O_RDWR | O_CREAT | O_EXCL , 0600, 
			 &fil_attrs)) == (mqd_t) -1){
	
	perror("mq_open"); 
	exit(1);
      }
    }
  }
}




/****************************************************************/
/* creer la socket de connexion                                 */
/****************************************************************/
void init_socket_connexion(){
  /* nom de la socket de connexion */
  struct sockaddr_in socket_in;

  /* nettre a zero les octets de socket_in */
  memset((char *)&socket_in, 0,sizeof(socket_in));
  /* @IP local*/
  socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
  /* le port d'ecoute */
  socket_in.sin_port = htons(PORTSERV);
  /* domaine de communication (internet...)*/
  socket_in.sin_family = AF_INET;
  /* creation de socket de connexion */
  if ((socket_connexion = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("socket");
    exit(1);
  }
  
  /* nommer la socket de connexion */
  if ((bind(socket_connexion, (struct sockaddr *) &socket_in, sizeof(socket_in))) == -1){
    perror("bind");
    exit(2);
  }
     
  /* creation de la file d'attente des requetes de connexion */
  if ((listen (socket_connexion, TAILLE_FILE)) == -1){
    perror("listen");
    close (socket_connexion);
    exit(3);
  }
}




/****************************************************************/
/* retourne la socket de communication liant le                 */
/* serveur au nouveau client. Quitte le programme               */
/* si une erreur est rencontree                                 */
/****************************************************************/
int wait_client(){
  
  int scom;
  
  /* le nom de l'adresse du client et sa taille */ 
  struct sockaddr_in src;  
  int fromlen = sizeof (src);  
  
  if ((scom = accept(socket_connexion, 
		     (struct sockaddr *)&src, 
		     (socklen_t *)&fromlen )) == -1){
    perror("accept");
    close (socket_connexion);
    exit(4);
  }
  return scom;
}




/****************************************************************/
/* la thread qui s'occupera de diffuser les messages            */
/* de la file de messages a tous les connectes                  */
/****************************************************************/
void start_thread_diffusion_messages(){
  /* creation de la thread d'envoie       */
  /* cette thread s'occupera de diffuser  */
  /* les messages de la file de messages */
  if (pthread_create((pthread_t *)&thread_envoie, NULL, 
		     diffuser_messages, (void *)&fil) != 0){
    fprintf(stderr,"[ERROR] pthread_create a rencontre un probleme\n");
    close (socket_connexion);
    close(fil);
    exit(5);
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
/* traitement a faire lors de la connexion d'un client alors    */
/* le serveur est plein */
/****************************************************************/
void server_full(int scom){
  chat_request reject_msg;
  set_requete(&reject_msg, "SYSTEM", "SERVER_FULL");
  envoyer_reponse(scom, &reject_msg);
}




/****************************************************************/
/* la thread qui s'occupe du client connecte                    */
/* via la socket scom */
/* la fonction renvoie -1 lorsque il n'y a plus de place sur le */
/* serveur. 1 lorsque tout va bien */
/****************************************************************/
int new_thread_for_client(int scom){
  int i;
  
  /* entree libre de tab_connectes */
  if ( (i = get_free_index()) == -1){
    fprintf(stderr,"[ERROR] Le chat est complet\n");
    server_full(scom);
    return -1;
  }
  
  /* incrementer le nombre des connectes */
  pthread_mutex_lock(&mutex_nb_connectes);
  nb_connectes++;
  pthread_mutex_unlock(&mutex_nb_connectes);
  /*
    fprintf(stderr, "DEBUG new_thread_for_client: i = %d\n", i);
  */
  tab_connectes[i].scom = scom;
  /*
    fprintf(stderr, "DEBUG new_thread_for_client: som = %d\n",
    tab_connectes[i].scom);
  */
  
  /* creation d'une thread qui traite la requete */  
  if  (pthread_create ((pthread_t *)&(tab_connectes[i].tid), NULL,
		       traitement_req, (void *)&(tab_connectes[i].scom)) != 0){
    fprintf(stderr,"[ERROR] pthread_create a rencontre un probleme\n");
    close (socket_connexion);
    exit(5);
  }
  
  /* rendre la thread detachee */
  /* la thread liberera elle meme ses ressources a la */
  /* fin de son execution */
  if(pthread_detach(tab_connectes[i].tid) != 0){
    perror("pthread_detach");
    close (socket_connexion);
    exit(1);
  }
  return 1;
}




/****************************************************************/
/* entree du programme                                          */
/****************************************************************/
int main(int argc, char * argv[]){
  
  /*--------- declarations ---------*/
  int socket_communication;  

  /* mettre en place le deroutement */
  init_deroutement();
  
  /* demarrage du serveur */
  fprintf(stderr,"Starting server...\n");
  
  init_socket_connexion();
  
  /* transfomer le serveur en demon */
  if (fork() != 0) exit (0);

  /* le serveur est up */ 
  fprintf(stderr,"Starting server...OK\n");
  
  /* mecanisme de diffusion de messages */
  init_file_messages();
  
  /* la thread de diffusion des messages */
  start_thread_diffusion_messages();
  
  /* attente des requetes des clients */
  while(1){ 
    /* creation de socket de communication */
    /* nous allons creer une thread pour chaque client */
    socket_communication = wait_client();
    /* creer une nouvelle thread qui s'occupera de notre */
    /* chere client ;) */
    if (new_thread_for_client(socket_communication) == -1){
      continue;
    }
  }
  
  return 0;    
}


