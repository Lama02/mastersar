/* exo2-2.c */
/* Ce programme presente une solution de l'exo 2-2 en           */
/* utilisant sigprocmask                                        */
/* Solution proposee : il suffit de masquer le signal SIGTERM   */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define SEC 20
#define NMAX 3 /* nombre maxi de SIGINT necessaire pour arreter le prog */

/* nombre de SIGINT recu */
int n=0; 


void sig_hand(int sig){
  switch (sig) {
  case SIGALRM: printf("%ds se sont ecoulees.\n", SEC); exit(0);
  case SIGTERM: fprintf(stderr,"On ne devrait pas voir ce message car le signal SIGTERM est masque\n"); exit(1);
  case SIGINT:  if (n<NMAX-1) n++; else exit(0); 
  default: return;
  }
}


int main (int argc, char * argv[]){
  
  /* definir les signaux bloquees */
  sigset_t sig_set, old_sig_set;
  struct sigaction action;
  
  /* Signaux a masquer */

  /* Tous*/
  sigfillset(&sig_set);

  /* sauf ces trois */
  sigdelset(&sig_set, SIGINT);
  sigdelset(&sig_set, SIGALRM);
  sigdelset(&sig_set, SIGTERM);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  sigprocmask(SIG_SETMASK, &sig_set, &old_sig_set);
  
  /* ici nous allons masquer le signal SIGTERM aussi */
  sigaddset(&sig_set, SIGTERM);
  
  /* mettre a jour la liste des signaux masques */
  sigprocmask(SIG_SETMASK, &sig_set, &old_sig_set);
  
  /* changement de traitement */
  action.sa_flags = 0;
  action.sa_handler = sig_hand;

  /* le handler a executer dans le cas ou on */
  /* receptionne SIGINT ou SIGALRM           */
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGALRM, &action, NULL);

  /* normalement nous allons pas recevoir SIGTERM      */
  /* car il est masque, mais l'appel de la sigaction   */
  /* suivante est une facon de tester notre code       */ 
  sigaction(SIGTERM, &action, NULL);
  
  alarm(SEC);
  while (1){
    /* en attente des signaux non masques */
    /* c'est a dire SIGINT et SIGALRM     */
    sigsuspend(&sig_set);
  }
  
  return EXIT_SUCCESS;
}
