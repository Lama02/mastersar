#define _PSOX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

/* le path vers gcc */
#define CC_PATH "/usr/bin/gcc"
#define CC "gcc"
/*
#define CFLAGS  "-Wall -ansi -pedantic -O" 
*/
#define CFLAGS  "-Wall"
 
int main(int argc, char * argv[]){
    
  pid_t fils;
  int i=0;
  printf("=== Compilation des sources ===\n");
  printf("argc = %d\n", argc);
  while (i < argc -1){
    if ( (fils=fork()) == 0 ){
      /* si je suis dans le fils */
      /* je lance la compilation du fichier */
      printf("Compilation du fichier %s ...\n",argv[i+1]); /* le backslach sera consomme par execl */
      execl(CC_PATH, CC, CFLAGS, "-c", argv[i+1], NULL);
      /* Si on revient ici alors execl s est mal executee */
      printf("Erreur: execl\n");
      exit(1);
    }else{
      /* si je suis dans le pere */
      i++;
    }
  }

  
  
  /* on a attend la mort des tous les fils */
  i=0;
  while(i< argc-1){
    wait(NULL);
    i++;
  }
  printf("=== Edition des liens ... ===\n");
  return 0;

}
