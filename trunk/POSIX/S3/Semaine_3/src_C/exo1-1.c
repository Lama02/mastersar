#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1


int main (int argc, char * argv[]){
  
  /* definir les signaux bloquees */
  sigset_t sig_set;
  int i;

  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGINT);
  sigaddset(&sig_set, SIGQUIT);
  
  /* masquer les signaux definies dans l ensemble sig_set */
  sigprocmask(SIG_SETMASK, &sig_set, NULL);
  
  sleep(10);
  
  
  /* lister les signaux pendants */
  if (sigpending (&sig_set) == -1)
    return EXIT_FAILURE;
  

  for (i=0; i<NSIG ; i++){
    if (sigismember(&sig_set, i) == 1)
      if (i == SIGINT)
	printf("\nSIGINT\n");
      else
	if (i == SIGQUIT)
	  printf("SIGQUIT\n");
  }

   
  return EXIT_SUCCESS;
}
