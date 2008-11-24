/* exo2-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>


#define N 5

/* chaque thread ajoutera a cette variable la */
/* valeur generee aleatoirement */
int somme_alea;

/* mutex pour proteger la var globale somme_alea */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

/* fonction lancee par chaque thread */
void* thread_rand(void * arg){
  /* generer la valeur aleatoire */
  int random_val = (int) ((float) 10 * rand() /(RAND_MAX + 1.0));

  /* afficher la valeur aleatoire */
  printf("Mon num d'ordre : %d \t mon tid %d \t valeur generee : %d\n", (*(int *)arg), (int)pthread_self(), random_val);
  
  /* poser un verrou */
  pthread_mutex_lock(&mutex);
  /* ajouter la valeur generee a somme_alea */
  somme_alea += random_val;
  /* enlever le verrou */
  pthread_mutex_unlock(&mutex);
  
  pthread_exit((void *)0); 
}


int main(int argc, char * argv[]){

  int i, p, status;
  /* pour regler le probleme du pointeur vers la */
  /* meme case memoire &i */
  int tab[N];
  
  /* tableau dans lequel on recupere les id des threads*/
  pthread_t tid[N];

  /* initialisations */
  somme_alea = 0;
  
  srand(time(NULL));

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
  }

  /* afficher la somme des valeurs generees par les threads creees */
  printf("La somme des valeurs generees par les threads est : %d \n", somme_alea);
  
  return EXIT_SUCCESS;
}
