#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define NBSEC 20

int main (int argc, char * argv[]){
  
  /* definir les signaux bloquees */
  sigset_t sig_set, old_sig_set;
  int i;

  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGINT);
  sigaddset(&sig_set, SIGQUIT);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  /* sauvegarder le masque du systeme dans old_sig_set */
  /* avant de l ecraser avec la valeur de sig_set */
  sigprocmask(SIG_SETMASK, &sig_set, &old_sig_set);
  
  sleep(NBSEC);
  
  
  /* lister les signaux pendants */
  if (sigpending (&sig_set) == -1)
    return EXIT_FAILURE;
  

  for (i=0; i<NSIG ; i++){
    if (sigismember(&sig_set, i) == 1)
      {
	if (i == SIGINT)
	  printf("\nSIGINT\n");
	else
	  {
	  if (i == SIGQUIT)
	    printf("SIGQUIT\n");
	  }
      }
  }
  
  
  /* effacer le masque des signaux pour SIGINT et SIGQUIT */
  sigprocmask(SIG_SETMASK, &old_sig_set, NULL);
  
  /* cette partie du code ne sera pas traitee dans le cas ou */ 
  /* ou on avant envoye un des dignaux SIGINT ou SIGQUIT, car */ 
  /* les signaux masquees seront maintenant pris en compte. */
  
  printf("Reprise apres demasquage\n");
  
  return EXIT_SUCCESS;
}
