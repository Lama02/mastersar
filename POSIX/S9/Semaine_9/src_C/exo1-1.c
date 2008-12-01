#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>

#define N 5


void* thread_rand(void * arg){
  int cod_ret = *((int *)arg); 
  printf("voici mon num d'ordre : %d et mon tid %d \n", cod_ret, (int)pthread_self());
  pthread_exit((void *) (cod_ret*7) ); 
}


int main(int argc, char * argv[]){

  int i, p, status;
  /* pour regler le probleme du pointeur vers la */
  /* meme case memoire */
  int tab[N];
  
  /* tableau dans lequel on recupere les id des threads*/
  pthread_t tid[N];
  
  for(i=0; i<N; i++){
    tab[i]=i;
    if((p=pthread_create(&(tid[i]), NULL, thread_rand, &tab[i])) != 0){
      perror("pthread_create \n");
      exit(1);
    }
  }
  
  /* attendre la fin de toutes les threads creees */
  for(i=0; i<N; i++){
   if(pthread_join(tid[i],(void**) &status) != 0){
      perror("pthread_join"); 
      exit(1);
    }
    else
      printf("Thread %d fini avec status : %d \n",i,status);
  }

  
  return EXIT_SUCCESS;
}
