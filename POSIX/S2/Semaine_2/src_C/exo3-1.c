#define _PSOX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>

/* le path vers gcc */
#define CC_PATH "/usr/bin/gcc"
#define CC "gcc"
/*
#define CFLAGS  "-Wall -ansi -pedantic -O" 
*/
#define CFLAGS  "-Wall"
 

/* 
   si src=test 
   dest deviendra teso 
*/
void echange(char * dest, char * src, int taille){
  strncpy(dest, src, taille);
  dest[taille-1]='o';
}



int main(int argc, char * argv[]){
  
  /* contiendra les nom des fichiers objets */
  char * tab_fils[]; 
  
  /* contiendra le nom du fichier d objet en cours de compilation */
  /* cette variable sera ecrase a chaque fois qu un nouveau fichier */
  /* source est traite */
  char * dest; 
  
  pid_t fils;
  int i=0;
  /* taille du nom du fichier */
  int taille; 
  /* 
     faire les substitutions .c -> .o et mettre le 
     dans un tableau
  */
  while (i < argc -1){
    taille = strlen(argv[1]);
    dest = malloc(taille+1 * sizeof(char)); /* il faut oublier le \0 */
    echange (dest, argv[1], taille); 
    
    
    i++;
  }
  printf("=== Compilation des sources ===\n");
  printf("argc = %d\n", argc);
  while (i < argc -1){
    if ( (fils=fork()) == 0 ){
      /* si je suis dans le fils */
      /* je lance la compilation du fichier */
      printf("Compilation du fichier %s ...\n",argv[i+1]); /* le backslach sera consomme par execl */
      
      /* stocker le nom du fichier dans un tab_fils */
      dest = malloc(taille+1 * sizeof(char)); /* il faut oublier le \0 */
      echange (dest, argv[1], taille); 

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
