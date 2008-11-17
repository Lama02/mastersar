#define _POSIX_SOURCE 1

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "string.h"
#include <signal.h>

#define ECRIRE 1
#define LIRE 0

#define TAILLE_SEG_MEMOIRE sizeof(char)

sem_t *sem_id_lire , *sem_id_ecrire;
int shm_id;
char *adr_att; 

void supprime_shm_sem(){
  /* fermer et detruire les semaphores */
  sem_close(sem_id_lire);
  sem_close(sem_id_ecrire);
  sem_unlink("sem_lire");
  sem_unlink("sem_ecrire");
  
  /* detacher le segment de memoire */
  munmap(adr_att, TAILLE_SEG_MEMOIRE);
  
  /* detruire le segment de memoire partage */
  shm_unlink("mon_shm");
}


void sig_hand(int sig){
  if (sig == SIGINT){  
    
    /* fermer et detruire les semaphores */
    /* detacher le segment de memoire */
    /* detruire le segment de memoire partage */
    supprime_shm_sem();
    exit (1);
  }
}




int main(int argc, char* argv[]){
  struct sigaction action;
  

  char char_lu;
  pid_t fils;
  
  /* Creation de semaphores avec les initialisations adequates */
  if ( (sem_id_lire = sem_open("sem_lire", O_CREAT | O_RDWR, 0600, LIRE)) == NULL){
    perror("sem_open");
    return 1;
  }
  
  if ( (sem_id_ecrire = sem_open("sem_ecrire", O_CREAT | O_EXCL |  O_RDWR, 0600, ECRIRE)) == NULL){
    /* fermer et detruire le semaphore deja cree */
    sem_close(sem_id_lire);
    sem_unlink("sem_lire");
    
    perror("sem_open");
    return 1;
  }
  
  /* Creation d'un segment de memomire partagee*/  
  if ( (shm_id = shm_open("mon_shm", O_CREAT | O_RDWR, 0600)) == -1){
    /* fermer et detruire les semaphores */
    sem_close(sem_id_lire);
    sem_close(sem_id_ecrire);
    sem_unlink("sem_lire");
    sem_unlink("sem_ecrire");
  
    perror("shmget");
    return 1;
  }
  
  /* allocation de la zone memoire pour le segment de memoire partagee */
  /* fraichement cree */
  if ( (ftruncate(shm_id, (off_t)TAILLE_SEG_MEMOIRE)) == -1){
    /* TODO Detruire le seg partage */
    /* fermer et detruire les semaphores */
    /* detacher le segment de memoire */
    /* detruire le segment de memoire partage */
    supprime_shm_sem();
    
    perror("ftruncate");
    return 1;
  }
  
  
  /* attacher le segment memoire shm_id au processus */
  if ( (adr_att = mmap(NULL, TAILLE_SEG_MEMOIRE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED){
    perror("mmap");
    /* TODO detruire les semaphores */
    /* TODO detruire le segment de memoire partage */
    /* fermer et detruire les semaphores */
    supprime_shm_sem();
    
    return 1;
  }
  
  
  /* mise en place du nouveau comportement */
  /* du processus lors de la recepetion du */
  /* signal SIGINT */

  /* changement de traitement */
  action.sa_flags = 0;
  action.sa_handler = sig_hand;
  sigaction(SIGINT, &action, NULL);
  
  
  switch(fils=fork()){
    
  case 0:
    while(1){
      if (sem_wait(sem_id_lire) == -1){
	/* fermer et detruire les semaphores */
	/* detacher le segment de memoire */

	supprime_shm_sem();
       
	perror("sem_wait");
	return 1;
      }
      char_lu = adr_att[0];
      if (sem_post(sem_id_ecrire) == -1){
	/* TODO detruire */
	/* fermer et detruire les semaphores */
	/* detacher le segment de memoire */
	/* detruire le segment de memoire partage */
	supprime_shm_sem();
	
	perror("sem_post");
	return 1;
      }
      printf("%c",toupper(char_lu));      
    }
  default:
    while(1){
      if (sem_wait(sem_id_ecrire) == -1){
	/* TODO detruire */
	/* fermer et detruire les semaphores */
	/* detruire le segment de memoire partage */
	supprime_shm_sem();
	
	perror("sem_wait");
	return 1;
      }
      scanf("%c",adr_att);
      if (sem_post(sem_id_lire) == -1){
	/* TODO detruire */
	/* fermer et detruire les semaphores */
	/* detruire le segment de memoire partage */
	supprime_shm_sem();
	perror("sem_post");
	return 1;
      }
    }
  }


  /* fermer et detruire les semaphores */
  /* detacher le segment de memoire */
  /* detruire le segment de memoire partage */
  supprime_shm_sem();
  
  return EXIT_SUCCESS;

}
