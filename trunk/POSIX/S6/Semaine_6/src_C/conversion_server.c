/*  */
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

#define BUFMAX sizeof(conversion_message)*NB_CONVERTERS

 int  fd_read, fd_write;

void sig_hand(int sig){
  
  if(sig==SIGINT){
    printf("Je ferme et je dettruit les tubes \n \n");
    close(fd_read);
    close(fd_write);
    /* unlik(argv[1]);*/
    /* unlik(argv[2]);*/
  }
  exit(3);
}


int main(int argc, char * argv[]){
  conversion_message req[1];
  int num_conveters, n;
  results_array tab_res;
  struct sigaction action;
  sigset_t sig_set;

  
  /* Signaux a masquee */
  /* Tous*/
  sigfillset(&sig_set);
  /* sauf SIGINT */
  sigdelset(&sig_set, SIGINT);

  sigprocmask(SIG_SETMASK, &sig_set, NULL);

  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = sig_hand;

  /* pour les signaux SIGINT */
  /* on applique le traitement definie par */
  /* la fonction sig_hand */
  sigaction(SIGINT, &action, NULL);


  /* Nombre d arguments */
  if (argc != 3){
    fprintf(stderr, "Erreur: Server nombre d'argument invalid.\n");
    exit(1);
  }

  
  /* Creation du premier tube nomme */
  if (mkfifo(argv[1],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }
  
  /* Creation du deuxieme tube nomme */
  if (mkfifo(argv[2],S_IRUSR|S_IWUSR) == -1 ){
    fprintf(stderr,"Erreur : mkfifo\n");
    exit(1);
  }

  printf("tube1 : %s, tube2 : %s \n",argv[1],argv[2]);

  sleep(4);
  /* ICI SURVIENT L'ERREUR*/  
  /* ouvrir en lecture */
  if ((fd_read = open(argv[1], O_RDONLY)) == -1) {
    fprintf(stderr,"Erreur : open\n");
    exit(2);
  }

  printf(" SERVER : apres ouverture du tube req en lecture \n \n");
  
  /* nous allons attendre qu'un client nous envoie une requete */
  while (1){

  printf("SER ds la boucle \n");    

    if((n=read(fd_read,&req,sizeof(conversion_message)))==-1){
      fprintf(stderr,"Erreur : read \n");
      exit(2);
    }
    /* on test si il n'y a pas d'ecrivain */
    
    /* if(n==0){
      close(fd_read);
      open(argv[1], O_RDONLY);
      continue; 
      }*/
    
    /* ici on convertit la requete */
    
    for (num_conveters=0 ; num_conveters < NB_CONVERTERS ; num_conveters++){
  
      /* on lance une requete de conversion pour chaque Devise cible (num_conveters) */
      /* le resultat est stocker dans le tableau results_array definie dans la biblio */
     
      handle_conversion_request(req[0], &tab_res[num_conveters], num_conveters);
    }
   
    /* ouverture en ecriture */

    if ((fd_write = open(argv[2], O_WRONLY)) == -1) {
      fprintf(stderr,"Erreur : open du write\n");
      exit(1);
    }
   
    write(fd_write,tab_res,BUFMAX);

    close(fd_write);
  }
  unlik(argv[1]);
  unlik(argv[2]);
  return 0;
}

