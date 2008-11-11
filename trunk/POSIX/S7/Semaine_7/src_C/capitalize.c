#define _POSIX_SOURCE 1

#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <signal.h>

#define ECRIRE 0
#define LIRE 1

#define TAILLE sizeof(char)

struct sembuf operation;

int sem_id;
int shm_id;

/* pour stocker les infos sur le segment de memoire partage */
struct shmid_ds *buf_shm;
/* pour stocker les infos sur le semaphore */
struct semid_ds *buf_sem_lire;
struct semid_ds *buf_sem_ecrire;



/* Primitive P() sur sémaphore*/
void P (int sem){
  operation.sem_num = sem;
  operation.sem_op = -1;
  operation.sem_flg = SEM_UNDO;
  if (semop (sem_id, &operation, 1) < 0){
    perror("semop P");
    exit(1);
  }
}


/* Primitive V() sur sémaphore*/
void V (int sem){
  operation.sem_num = sem;
  operation.sem_op = 1;
  operation.sem_flg = SEM_UNDO;
  if (semop (sem_id, &operation, 1) < 0){
    perror("semop V");
    exit(1);
  }
}

void sig_hand(int sig){
  if (sig == SIGINT){  
    printf("LOLOLOLO\n");
    /* detruire les semaphores */
    semctl(sem_id, LIRE, IPC_RMID, buf_sem_lire);
    semctl(sem_id, ECRIRE, IPC_RMID, buf_sem_ecrire);
    
    /* detruire le segment de memoire partage */
    shmctl(shm_id, IPC_RMID,buf_shm);
  }
}


int main(int argc, char* argv[]){
  struct sigaction action;
  
  /* pour stocker les infos sur le segment de memoire partage */
  buf_shm = (struct shmid_ds *) malloc(sizeof(struct shmid_ds));
  /* pour stocker les infos sur le semaphore */
  buf_sem_lire = (struct semid_ds *) malloc(sizeof(struct semid_ds));
  buf_sem_ecrire = (struct semid_ds *) malloc(sizeof(struct semid_ds));
  
  /* struct shmid_ds *buf; */
  key_t cle; 
  char *adr_att; 
  char char_lu;
  pid_t fils;
  char path[14]="mem_par";
  char code = 'M';


  cle=ftok(path,code);
  
  /* Creation d'une entree dans la table d'ensemble de semaphores*/
  if ( (sem_id = semget(cle, 2,  0666|IPC_CREAT)) == -1){
    perror("semget");
    return 1;
  }
    
  /* initialisation des deux semaphores */
  semctl(sem_id, ECRIRE, SETVAL, 1);
  semctl(sem_id, LIRE, SETVAL, 0);

  /* Creation d'une entree dans la table des segments de memomire partagee*/  
  if ( (shm_id = shmget(cle, TAILLE, 0666|IPC_CREAT)) == -1){
    perror("shmget");
    return 1;
  }
  
  if ( (adr_att = shmat(shm_id, 0, 0600)) == (char *)-1){
    perror("shmat");
    /* detruire les semaphores */
    semctl(sem_id, LIRE, IPC_RMID, buf_sem_lire);
    semctl(sem_id, ECRIRE, IPC_RMID, buf_sem_ecrire);

    
    /* detruire le segment de memoire partage */
    shmctl(shm_id, IPC_RMID,buf_shm);
    
    return 1;
  }
  
  
  /* mettre les info (du systeme) du segment */
  /* de memoire partagee dans buf_shm */
  shmctl(shm_id,IPC_STAT,buf_shm);
  /* mettre les info (du systeme) du semaphore */
  /* dans buf_sem_ecrire et buf_sem_lire */
  semctl(sem_id, LIRE, IPC_STAT, buf_sem_lire);
  semctl(sem_id, ECRIRE, IPC_STAT, buf_sem_ecrire);

  

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
      P(LIRE);
      char_lu = adr_att[0]; 
      V(ECRIRE);
      printf("%c",toupper(char_lu));
      
    }
  default:
    while(1){
      P(ECRIRE);
      scanf("%c",adr_att);
      V(LIRE);      
    }
  }

  /* detruire le semaphore */
  semctl(sem_id, LIRE, IPC_RMID, buf_sem_lire);
  semctl(sem_id, ECRIRE, IPC_RMID, buf_sem_ecrire);
  
  /* detruire le segment de memoire partage */
  shmctl(shm_id, IPC_RMID,buf_shm);
  return EXIT_SUCCESS;

}
