/* exo4-1.c */
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

/* nombre de threads executees */
int cpt;

/* mutex pour proteger la var globale somme_alea */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

/* pour attendre l'exec de toutes les threads */
pthread_mutex_t mutex_cpt = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond_cpt = PTHREAD_COND_INITIALIZER; 


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
  
  /* pour connaitre la derniere thread */
  pthread_mutex_lock(&mutex_cpt);
  cpt++;

  /* si je suis la derniere thread */
  if (cpt == N){

    /* je reveille la thread d'affichage */
    pthread_cond_signal(&cond_cpt);
  }
  pthread_mutex_unlock(&mutex_cpt);
  
  pthread_exit((void *)0); 
}


/* fonction lancee par la thread main, ceci affiche la */
/* somme des valeurs generees */
void * print_thread(void * arg){
  pthread_mutex_lock(&mutex_cpt);

  /* attente de l'exec de toutes les threads */
  pthread_cond_wait(&cond_cpt, &mutex_cpt);
  pthread_mutex_unlock(&mutex_cpt);

  /* afficher la somme des valeurs generees par les threads creees */
  printf("La somme des valeurs generees par les threads est : %d \n", somme_alea);

  pthread_exit((void *)0);
}

int main(int argc, char * argv[]){
  
  int i, p,status;
  /* pour regler le probleme du pointeur vers la */
  /* meme case memoire &i */
  int tab[N];
  
  /* tableau dans lequel on recupere les id des threads*/
  pthread_t tid[N];
  pthread_t pt_print_tid;

  /* initialisations */
  somme_alea = 0;
  cpt = 0;
  
  srand(time(NULL));

  /* thread d'affichage de la somme des valeurs generees */
  if (pthread_create(&pt_print_tid, NULL, print_thread, NULL) != 0){
    perror("pthread_create");
    exit(1);
  }
  
  for(i=0; i<N; i++){
    tab[i]=i;
    if((p=pthread_create(&(tid[i]), NULL, thread_rand, &tab[i])) != 0){
      perror("pthread_create");
      exit(1);
    }

    if(pthread_detach(tid[i]) != 0){
      perror("pthread_detach");
      exit(1);
    }
  }
  
  /* attendre l'affichage du resultat */
  if(pthread_join( pt_print_tid,(void**) &status) != 0){
    perror("pthread_join");
    exit(1);
  }
  
  
  return EXIT_SUCCESS;
}
