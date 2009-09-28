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
#define CFLAGS  "-Wall -ansi" 
*/
#define CFLAGS  "-Wall"
/* nom du binaire */
#define PROG  "exec_exo3-1"

/* 
   si src=test 
   dest deviendra teso 
*/
void echange(char * dest, char * src, int taille){
  strncpy(dest, src, taille);
  dest[taille-1]='o';
}



int main(int argc, char * argv[]){
  
  /* 
     les 4 premiere cas contiendront 
     - CC
     - CFLAGS
     - -o
     - prog
     la derniere cas contiendra NULL
     tout les reste contiendra les nom des
     fichiers objets generes  
  */
  char * tab_fils[argc+4]; 
  
  /* contiendra le nom du fichier d objet en cours de compilation */
  /* cette variable sera ecrase a chaque fois qu un nouveau fichier */
  /* source est traite */
  char * dest; 
  
  pid_t fils;
  int i=0;
  /* taille du nom du fichier */
  int taille; 
  
  if (argc < 2 ){
    fprintf(stderr,"Erreur. Nombre d'arguments invalide.\n");
    fprintf(stderr, "usage : %s src.c ...\n",argv[0]);
    return 1;
  }
  

  /* 
     faire les substitutions .c -> .o et mettre les 
     resultats dans un tab_fils
  */
  while (i < argc -1){
    taille = strlen(argv[i+1]);
    dest = malloc(taille+1 * sizeof(char)); /* il faut pas oublier le \0 */
    /* transformer src.c en src.o */
    echange (dest, argv[i+1], taille); 
    /* sauvegarder la reference vers le nom du fichier objet*/
    /* car elle sera perdu a la prochaine iteration */
    tab_fils[i+4]=dest; 
    i++;
  }
  
  printf("=== Compilation des sources ===\n");
  i=0;
  while (i < argc -1){
    if ( (fils=fork()) == 0 ){
      /* si je suis dans le fils */
      /* je lance la compilation du fichier */
      printf("Generation du fichier %s ...\n",tab_fils[i+4]); /* le backslach sera consomme par execl */      
      /* gcc -Wall -o path/toto.o -c path/toto.c */
      execl(CC_PATH, CC, CFLAGS, "-o", tab_fils[i+4], "-c", argv[i+1], NULL);
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
  tab_fils[0]=CC;
  tab_fils[1]=CFLAGS;
  tab_fils[2]="-o";
  tab_fils[3]=PROG;
  tab_fils[argc+4]=NULL;
  printf("L'edition des liens est en cours...");
  if (!( fils=fork() )){
    /* je suis dans le fils*/
    execv(CC_PATH,tab_fils);
    fprintf(stderr, "*** Erreur pendant l'edition des liens ***\n");
  }
  
  /* on attend la mort du fils */
  wait(NULL);
  printf("OK\n");
  printf("L'edition des liens s'est bien deroulee. L'executable genere est %s\n",PROG);
  return 0;

}
