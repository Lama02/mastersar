/* "server.c" */
#include "chat.h"




/* liste des connectes */
un_connecte tab_connectes[MAX_CONNECTES];

/* nombre de connecte */
int nb_connectes = 0;

/* la socket de connexion */
int socket_connexion ;

/* identifiant de la file de message */
mqd_t fil;

/* thread d'envoie des messages */
pthread_t thread_envoie;

/* mutex pour proteger la liste de messages */
pthread_mutex_t mutex_liste_message = PTHREAD_MUTEX_INITIALIZER; 
/* mutex pour proteger la variable nb_connectes */
pthread_mutex_t mutex_nb_connectes  = PTHREAD_MUTEX_INITIALIZER; 
/* mutex pour proteger la liste des connectes */
pthread_mutex_t mutex_tab_connectes  = PTHREAD_MUTEX_INITIALIZER; 



/* stoper la communication */
void stop_communication(int scom){
  /* deconnexion : arreter les envoies/receptions */
  shutdown(scom, 2);
  close(scom);
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
  /* posons un verrou sur la liste */
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].valid == 1){
      stop_communication(tab_connectes[i].scom);
    }
  }
  /* enlever le verrou sur la lise */
  pthread_mutex_unlock(&mutex_tab_connectes);

  /* fermer et supprimer la file de messages */
  mq_unlink("/file");
  close(fil);

  fprintf(stderr, "OK\n");
  exit(0);
}




/* TODO a enlever */
/* gestion des logs */
void add_log(char * log){
  fprintf(stderr,"[INFO] %s\n",log);
}




/* verifie si la requete est un JOIN */
/* retourne 1 si c'est le cas 0 sinon*/
int est_join(chat_request * req){
  if ( (strcmp(req->msg, "JOIN")) == 0 ) {
    /* c est un join*/
    return 1;
  }else{
    return 0;
  }
}




/* verifie si la requete est un QUIT  */
/* retourne 1 si c'est le cas 0 sinon */
int est_quit(chat_request req){
  if ( (strncmp(req.msg, "QUIT",4)) == 0 ) {
    /* c est un quit */
    return 1;
  }else{
    return 0;
  }
}




/* ajoute le client a la liste des connectes */
void add_connecte(int scom, char * pseudo){
  int i;
  pthread_mutex_lock(&mutex_tab_connectes);
  for (i=0; i< MAX_CONNECTES; i++){
    if (tab_connectes[i].scom == scom){
      strcpy(tab_connectes[i].pseudo, pseudo);
      fprintf(stderr,"[INFO] Ajout de %s a la liste des connectes\n", pseudo);
      break;
    }
  }
  pthread_mutex_unlock(&mutex_tab_connectes);
}




/* supprime le client de la liste des connectes */
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




/* verifie si le client peut se connecter au serveur   */
/* si c'est le cas, on lui reserve une place sinon la  */
/* thread appelante se termine et un message decrivant */
/* le probleme est envoye au client                    */
void connexion(int scom, char * pseudo){

  chat_request resp_to_cli;
  
  /* verifier si le nombre maxi de connectes n'est pas atteint */
  /* poser un verrou pour proteger nb_connectes */
  pthread_mutex_lock(&mutex_nb_connectes);
  if (nb_connectes >= MAX_CONNECTES) {
    /* pas la peine de trouver une place dans la liste des
       connecte. bye */
    /* enlever le verro */
    pthread_mutex_unlock(&mutex_nb_connectes);

    strcpy (resp_to_cli.pseudo, "SYSTEM");
    strcpy (resp_to_cli.msg, "JOIN_MAX_CONNECTIONS");
    if (write(scom, &resp_to_cli, sizeof(resp_to_cli)) == -1){
      perror("write");
      stop_communication(scom);
      pthread_exit((void *)1);       
    }
    
   fprintf(stderr, "[INFO] Rejet du client %s. plus de place sur le serveur\n",pseudo);
   /* deconnexion : arreter les envoies/receptions */
   stop_communication(scom);
   pthread_exit((void *)1);      
  }
  
  /* le verro n'est pas encore ete enleve */
  /* incrementer le nombre des connectes */
  nb_connectes++;
  /* enlever le verrou */
  pthread_mutex_unlock(&mutex_nb_connectes);
}




/* traitement a faire dans le cas ou le message */
/* recu correspond a un join                    */
void traitement_join(int scom, chat_request * req_from_cli){
  
  chat_request resp_to_cli;

  fprintf(stderr, "[INFO] Le client %s veut joindre le serveur\n",
	  req_from_cli->pseudo);
  
  
  /* peut-on se connecter ? */
  connexion(scom, req_from_cli->pseudo);
  
  /* le message a diffuser */    
  strcpy(resp_to_cli.pseudo,"SYSTEM");
  sprintf(resp_to_cli.msg,"%s a rejoint le chat",req_from_cli->pseudo);
  
  /* ajout du message dans la file des messages*/
  pthread_mutex_lock(&mutex_liste_message);
  if(mq_send(fil, (char *) &resp_to_cli , sizeof(chat_request), 1) == -1){
    perror("mq_send");
    stop_communication(scom);
    pthread_mutex_unlock(&mutex_liste_message);
    pthread_exit((void *)1);
  }
  pthread_mutex_unlock(&mutex_liste_message);
    
    
  /* ajouter le client dans la liste des participants */
  /* on a deja reserve notre place messieurs :) */
  add_connecte(scom, req_from_cli->pseudo);
  
  
  /* envoyer un acquittement pour la connexion au client */
  strcpy(resp_to_cli.pseudo, "SYSTEM");
  strcpy(resp_to_cli.msg, "JOIN_ACK");
  if (write( scom, &resp_to_cli, sizeof(resp_to_cli)) == -1){
    perror("write");
    stop_communication(scom);
    pthread_exit((void *)1);      
  }
  
  fprintf(stderr,"[INFO] Le client %s peut joindre le serveur\n", req_from_cli->pseudo);     
}



/* suppression de la liste des connectes */
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


/* Avertie tous les participants que le client */
/* pseudo est entrain de quitter le chat       */
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
  /* mettre le message dans la file */
  pthread_mutex_lock(&mutex_liste_message);
  if(mq_send(fil, (char *) &resp_to_cli, sizeof(chat_request), 1) == -1){
    perror("mq_send");
    stop_communication(scom);
    pthread_mutex_unlock(&mutex_liste_message);
    pthread_exit((void *)1);
  }
  pthread_mutex_unlock(&mutex_liste_message);

  fprintf(stderr, "[INFO] envoyer un QUIT_ACK a %s \n", pseudo);
  
  /* ici on envoie un acquittement pour la deconnexion au client */
  strcpy(resp_to_cli.pseudo, "SYSTEM");
  strcpy(resp_to_cli.msg, "QUIT_ACK");
  if (write(scom, &resp_to_cli, sizeof(resp_to_cli)) == -1){
    perror("traitement_quit - write");
    stop_communication(scom);
    pthread_exit((void *)1);      
  }
  
  fprintf(stderr, "[INFO] Arreter la communication avec le client %s\n", pseudo);
  /* deconnexion : arreter les envoies/receptions */
  stop_communication(scom);
  fprintf(stderr, "[INFO] fin communication avec %s. BYE\n", pseudo);
  pthread_exit((void *)0);
}



/* declaration de la fonction de traitement */
/* des requetes */
void * traitement_req(void * scom){

  /* requete envoyee par le client */
  chat_request req_from_cli;

  /* attendre que le client nous envoie une reqete */
  if ((read(*(int *) scom, &req_from_cli, sizeof(req_from_cli))) == -1){ 
    perror("read 33");
    stop_communication(*(int *)scom);
    pthread_exit((void *)1);      
  }
  
  
  /*-- traitement du message envoye par le client --*/
  /* le message correspond a JOIN */
  if (est_join(&req_from_cli) == 1){
    traitement_join(*(int *)scom, &req_from_cli);
  }else{
    /* le message rec un est pas un join              */
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
    if (read(*(int *) scom, &req_from_cli, sizeof(req_from_cli)) == -1){  
      perror("read 44");
      stop_communication(*(int *)scom);
      pthread_exit((void *)1);      
    }
    
    /* le message correspond a QUIT */
    printf(" message lu par le serveur : %s\n", req_from_cli.msg);
    printf(" resultat de le fct est_quit : %d \n",est_quit(req_from_cli));
    if (est_quit(req_from_cli) == 1){
      printf("Je suis dans est_quit \n ");
      traitement_quit(*(int*)scom, req_from_cli.pseudo);
    }else{  
      printf("Je ne suis pas dans est_quit \n"); 
      /* Message normal a diffuser a tous les autres clients */
      pthread_mutex_lock(&mutex_liste_message);
      if(mq_send(fil, (char *) &req_from_cli, sizeof(chat_request), 1) == -1){
	perror("mq_send");
	pthread_mutex_unlock(&mutex_liste_message);
	pthread_exit((void *)1);
      }
      pthread_mutex_unlock(&mutex_liste_message);
    }
  } 
}





/* cette fonction diffuse les messages se    */
/* trouvant dans la file des messages a tous */
/* les participants au chat */
void * diffuser_messages(){
  int i;
  /* le message a envoyer */
  chat_request resp_to_cli;

  while(1){
    if (mq_receive(fil, (char*) &resp_to_cli, 
		   sizeof(resp_to_cli)  , NULL ) == -1){
      perror("mq_receive"); 
      stop_server();
      pthread_exit((void *)1);
      }
    
    /* envoyer le message */
    pthread_mutex_lock(&mutex_tab_connectes);
    for (i=0; i< MAX_CONNECTES; i++){
      if (tab_connectes[i].valid == 1){ 
	if (write(tab_connectes[i].scom, &resp_to_cli, 
		  sizeof(resp_to_cli)) == -1){
	  perror("write");
	  stop_server();
	  pthread_mutex_unlock(&mutex_tab_connectes); 
	  pthread_exit((void *)1);      
	}
      }
    }
    pthread_mutex_unlock(&mutex_tab_connectes);
  }
}



/* retourne le premier index libre de la     */
/* table des connectes. -1 si plus de place  */
/* libre */
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


/* entree du programme */
int main(int argc, char * argv[]){
  
  /*--------- declarations ---------*/
  int socket_communication;
  
  /* nom de la socket de connexion */
  struct sockaddr_in socket_in;

  /* le nom de l'adresse du client et sa taille */ 
  struct sockaddr_in src;  
  int fromlen = sizeof (src);  
  
  int i; 

  /* pour le deroutement */
  sigset_t sig_set;
  struct sigaction action;
  
  /* pour la file des messages */
  struct mq_attr fil_attrs;
  
  /*--------- initialisations ---------*/
  
  /* nettre a zero les octets de socket_in */
  memset((char *)&socket_in, 0,sizeof(socket_in));
  /* @IP local*/
  socket_in.sin_addr.s_addr = htonl(INADDR_ANY);
  /* le port d'ecoute */
  socket_in.sin_port = htons(PORTSERV);
  /* domaine de communication (internet...)*/
  socket_in.sin_family = AF_INET;
  
  
  fil_attrs.mq_flags = 0;
  fil_attrs.mq_curmsgs = 0;
  fil_attrs.mq_maxmsg = 10;
  fil_attrs.mq_msgsize = sizeof(chat_request);
  
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
  if( (fil = mq_open("/file", O_RDWR | O_CREAT , 0600, 
		     &fil_attrs)) == (mqd_t) -1){
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

  /* transfomer le serveur en demon */
  if (fork() != 0) exit (0);

  /* creation de la thread d'envoie       */
  /* cette thread s'occupera de diffuser  */
  /* les messages de la file des messages */
  if (pthread_create((pthread_t *)&thread_envoie, NULL, 
		     diffuser_messages, (void *)&fil) != 0){
    fprintf(stderr,"pthread_create a rencontre un probleme\n");
    close (socket_connexion);
    close(fil);
    exit(5);
  } 
  
  /* attente des requetes des clients */
  while(1){ 
    /* creation de socket de communication */
    /* nous allons creer une thread pour chaque client */
    
    if ((socket_communication = accept(socket_connexion, 
				       (struct sockaddr *)&src, 
				       (socklen_t *)&fromlen )) == -1){
      perror("accept");
      close (socket_connexion);
      close(fil);
      exit(4);
    }
    
    /* */
    i = get_free_index();
    tab_connectes[i].scom = socket_communication;
    fprintf(stderr, "DEBUG i = %d\n", i);
    /* creation d'une thread qui traite la requete */  
    if  (pthread_create ((pthread_t *)&(tab_connectes[i].tid), NULL, traitement_req, (void *)&(tab_connectes[i].scom)) != 0) {
      fprintf(stderr,"pthread_create a rencontre un probleme\n");
      close (socket_connexion);
      close(fil);
      exit(5);
    }
    
    /* rendre la thread detachee */
    /* la thread liberera elle meme ses ressources a la */
    /* fin de son execution */
    if(pthread_detach(tab_connectes[i].tid) != 0){
      perror("pthread_detach");
      close (socket_connexion);
      close(fil);
      exit(1);
    }
  }
  
  return 0;    
}


