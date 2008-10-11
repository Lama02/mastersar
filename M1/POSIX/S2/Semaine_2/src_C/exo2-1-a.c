#define _PSOX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#define N 10


int main(){

  /* LANCEMENT ET ATTENTE DES PROCESSUS FILS */
  printf("=== Attente de la mort de tous les fils ===\n");
  
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
  
  /* on a attend la mort des tous les fils */
  i=0;
  while(i<N){
    wait(NULL);
    i++;
  }
  printf("Tout le monde est mort. c est moi le pere mais je meurs moi aussi\n");
  return 0;

}
