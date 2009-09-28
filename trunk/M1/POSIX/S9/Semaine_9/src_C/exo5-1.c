/* exo5-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>

#define N 5


/* pour attendre l'exec de toutes les threads */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

/* pour proteger la variable globale var */
pthread_mutex_t mutex_var = PTHREAD_MUTEX_INITIALIZER; 

int var;


int wait_barrier (int n){
  
  pthread_mutex_lock(&mutex_var); 
  var++;
  pthread_mutex_unlock(&mutex_var);
  
  if( var == n ){
    
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    
  }
  else{
    
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);   
    pthread_mutex_unlock(&mutex);
    
  }
  
  return 0;
  
}


void* thread_func (void *arg) {

  printf ("avant barriere\n");

  wait_barrier (N);
  
  printf ("apres barriere\n");
  
  pthread_exit ( (void*)0);

}


int main(int argc, char * argv[]){
  
  int i, p,status;

  /* pour regler le probleme du pointeur vers la */
  /* meme case memoire &i */
  int tab[N];
  
  /* tableau dans lequel on recupere les id des threads*/
  pthread_t tid[N];
  
  var=0;
  
  for(i=0; i<N; i++){
    tab[i]=i;
    if((p=pthread_create(&(tid[i]), NULL, thread_func,NULL)) != 0){
      perror("pthread_create");
      exit(1);
      
    }
    
  }
  
  /* attendre la fin de toutes les threads creees */
  for(i=0; i<N; i++){
    
    if(pthread_join(tid[i],(void**) &status) != 0){
      perror("pthread_join"); 
      exit(1);
    }
  }
  
  return EXIT_SUCCESS;
}
