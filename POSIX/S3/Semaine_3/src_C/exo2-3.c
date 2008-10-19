#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define NBSEC 5


void sig_hand(int sig){
  printf("j ai bien recu le signal du fils \n");
}


int main (int argc, char * argv[]){
  /* definir les signaux bloquees */
  sigset_t sig_set;
  struct sigaction action;
  pid_t fils;
  
  /* Signaux a masquee */
  /* Tous*/
  sigfillset(&sig_set);
  
  /* sauf un */
  sigdelset(&sig_set, SIGCHLD);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  sigprocmask(SIG_SETMASK, &sig_set,NULL);
  
  /* changement de traitement */
  action.sa_mask = sig_set;
  action.sa_flags = 0;
  action.sa_handler = sig_hand;
  sigaction(SIGCHLD, &action, NULL);
  
  if((fils=fork())==0){
    printf("je suis le fils : %d \n",getpid());
    exit(0);
  }
  
  /* le fils mourra bien avant que le pere */
  /* prendra compte de sa terminaison */
  sleep(NBSEC); 

  
  sigsuspend(&sig_set);
  return EXIT_SUCCESS;
}
