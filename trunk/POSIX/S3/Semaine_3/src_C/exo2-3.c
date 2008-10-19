#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define NBSEC 5


void sig_hand(int sig){
  printf("j ai bien recu le signal de mon fils, je me suicide...\n");
}


int main (int argc, char * argv[]){
  /* definir les signaux bloquees */
  sigset_t sig_set, old_sig_set;
  struct sigaction action;
  pid_t fils;
  
  /* Signaux a masquee */
  /* Tous*/
  sigfillset(&sig_set);
  
  /* sauf un */
  sigdelset(&sig_set, SIGCHLD);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  sigprocmask(SIG_SETMASK, &sig_set,NULL);
  
  /* nous allons masquer SIGCHLD aussi */
  sigaddset(&sig_set, SIGCHLD);
  /* appliquer le masque en sauvegardant l'ancien           */
  /* masque, cad celui ou SIGCHLD n'est pas masque, dans    */
  /* old_sig_set. Ceci nous sera utile lorsque nous aurions */
  /* besoin de demasquer SIGCHLD */
  sigprocmask(SIG_SETMASK, &sig_set,&old_sig_set);
  
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


  /* nous allons attendre les signaux definies dans */
  /* l'ensemble old_sig_set, cad le signal SIGCHLD*/
  sigsuspend(&old_sig_set);
  
  /* restauration du masque ou SIGCHLD n'est pas      */
  /* masque. Application de ce masque pour le process */
  /* courant: le pere */
  sigprocmask(SIG_SETMASK, &old_sig_set, NULL);

  
  return EXIT_SUCCESS;
}
