#define _PSOX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define N 10


int main(){

  /* LANCEMENT ET ATTENTE DES PROCESSUS FILS */
  printf("=== Attente de la mort d'un des fils ===\n");
  
  pid_t fils;
  int i=0;
  
  
  while (i < N){
    if ( (fils=fork()) == 0 ){
      /*si je suis dans le fils */
      printf("je suis le %d fils.\n",i);
      exit (0);
    }else{
      /*si je suis dans le pere */
      printf("oui mais c est moi le pere ici !!\n");
      i++;
    }
  }
  
  /* on a attend la mort d'un des fils */
    wait(NULL);
    printf("C'est moi le pere, un de mes fils est mort, je vais me suicider :-(\n");
  return 0;

}
