#include <stdio.h>

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define SEC 20
#define NMAX 3 /* nombre maxi de SIGINT necessaire pour arreter le prog */

int n=0; /* nombre de SIGINT recu */

void sig_hand(int sig){
  switch (sig) {
  case SIGALRM: printf("%ds se sont ecoulees.\n", SEC); exit(0);
  case SIGTERM: exit(0);
  case SIGINT:  if (n<NMAX-1) n++; else exit(0); 
  default: return;
  }
}


int main (int argc, char * argv[]){
  
  /* definir les signaux bloquees */
  sigset_t sig_set, old_sig_set;
  struct sigaction action;

  
  /* Signaux a masquee */

  /* Tous*/
  sigfillset(&sig_set);

  /* sauf ces trois */
  sigdelset(&sig_set, SIGINT);
  sigdelset(&sig_set, SIGALRM);
  sigdelset(&sig_set, SIGTERM);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  sigprocmask(SIG_SETMASK, &sig_set, &old_sig_set);
  
  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = sig_hand;

  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGALRM, &action, NULL);
  
  alarm(SEC);
  while (1){
    sigsuspend(&sig_set);
  }

  return EXIT_SUCCESS;
}
