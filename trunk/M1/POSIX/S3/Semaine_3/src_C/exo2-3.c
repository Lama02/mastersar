/*
 * exo2-3.c
 * Ce programme ne marche que pour un seul fils, car 
 * lorsque on masque un signal, SIGCHLD en l'occurrence, 
 * si on recoie plusieurs instances de ce signal une seul 
 * est retenue, toutes les autres sont rejetees
 */
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
  

  /** Signaux a masquer          **/

  sigemptyset(&sig_set);
  
  /* appliquons ce premier masque */
  /*  aucun signal n'est masque */
  sigprocmask(SIG_SETMASK, &sig_set,NULL);
  
  /* nous allons masquer SIGCHLD  */
  sigaddset(&sig_set, SIGCHLD);
  
  /* appliquer le masque en sovgardant la valeur de              */
  /* l'ancien masque dans old_sig_set c'est a dire               */
  /* la ou SIGCHLD n'est pas masque. Ceci nous permetra          */
  /* de le demasquer facilement plus tard (grace a sigsuspend()) */
  sigprocmask(SIG_SETMASK, &sig_set,&old_sig_set);
  
  if((fils=fork())==0){
    printf("je suis le fils  %d j'envoie un SIGCHLD a mon pere %d \n",getpid(),getppid());
    exit(0);
  }
  
  /* changement de traitement pour SIGCHLD */
  action.sa_flags = 0;
  action.sa_handler = sig_hand;
  sigaction(SIGCHLD, &action, NULL);

  /* le fils mourra bien avant que le pere */
  /* prendra compte de sa terminaison      */
  sleep(NBSEC); 
    
  /* Comme SIGCHLD est masque, meme si le fils meurt         */
  /* le signal SIGCHLD qu'il renvoyera a son pere sera       */
  /* mis en attente (signaux pendants)                       */
  /* ceci ne sera pas pris en compte que lorsqu'on demasque  */
  /* le signal. sigsuspend attend le signal et le demasque   */
  /* en masque     */
  /* signal SIGCHLD */
  
  /* nous allons attendre le signal SIGCHLD */  
    
  sigsuspend(&old_sig_set);
  
  
  return EXIT_SUCCESS;
}
