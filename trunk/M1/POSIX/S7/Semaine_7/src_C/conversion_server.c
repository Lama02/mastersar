#define _POSIX_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "converters.h"
#include "string.h"

#define BUFMAX sizeof(results_array) 

int fd_read, fd_write;
char * tube1, *tube2;

/* arrete le serveur et detruit les tubes crees */
void fin_serveur(){  
  fprintf(stderr, "Stopping server...");
  close(fd_read);
  close(fd_write);
  unlink(tube1);
  unlink(tube2);
  fprintf(stderr, "OK\n");
  
  exit(3);
}


int main(int argc, char * argv[]){
  int sig;
  conversion_message req[1];
  int num_conveters, n;
  results_array tab_res;
  struct sigaction action;
  sigset_t sig_set;
  conversion_message ligne_res;
  conversion_message req_du_pere;
  
  
  /* a la reception de n importe quel signal */
  /* on supprime les tubes puis on arrete le serveur */
  sigemptyset(&sig_set);
  sigprocmask(SIG_SETMASK, &sig_set, NULL);
  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = fin_serveur;
  for (sig=1; sig<= _NSIG; sig++) sigaction(sig, &action, NULL);

  /* Nombre d arguments */
  if (argc != 3){
    fprintf(stderr, "Erreur: Server nombre d'argument invalid.\n");
    exit(1);
  }

  /* stocker les nom des tubes */
  tube1 = argv[1];
  tube2 = argv[2];

  fprintf(stderr, "Starting server...");
  
  /* Creation du premier tube nomme */
  /* pour la lecture */
  if (mkfifo(argv[1],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }

  
  /* Creation du second tube nomme */
  /* pour ecrire */
  if (mkfifo(argv[2],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }


  fprintf(stderr, "OK\n");
    
  /* ouvrir en lecture */
  if ((fd_read = open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr,"Erreur : open\n");
    exit(2);
  }

  /* ouverture en ecriture */
  if ((fd_write = open(argv[2], O_WRONLY)) == -1) {
    fprintf(stderr,"Erreur : open du write\n");
    exit(1);
  }

  
  /* nous allons attendre qu'un client nous envoie une requete */
  while (1){
    if((n=read(fd_read,&req,sizeof(conversion_message)))==-1){
      fprintf(stderr,"Erreur : read \n");
      exit(2);
    }
    

    /* on test si il n'y a pas d'ecrivain */
     if(n==0){
       /* lorsque il n'y a pas de client on attend */
       close(fd_read);
       /* ouvrir en lecture */
       if ((fd_read = open(argv[1], O_RDONLY)) == -1) {
	 fprintf(stderr,"Erreur : open\n");
	 exit(2);
       }
      continue; 
      }

    
     /* ici on convertit la requete */
     /* nous allons creer un fils qui s'occupera de */
     /* la conversion vers une devise donnee */

     while ( (fork() == 0) && (num_conveters<NB_CONVERTERS) ){
       /* le fils */
       
       
       for (num_conveters=0 ; num_conveters < NB_CONVERTERS ; num_conveters++){
	 
	 /* on lance une requete de conversion pour la Devise cible (num_conveters) */
	 /* le resultat est stocker dans la variable ligne_res */
       handle_conversion_request(req[0], &ligne_res, num_conveters);
       
       /* TODO */
       /* ecrire ligne_res dans le segment de memoire partagee */
       
       
       exit(0);
     }
     
     /* le pere */
       
       
     
    write(fd_write,tab_res,BUFMAX);
  }
  return 0;
}

