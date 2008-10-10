#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*#ifndef N_FILS
#define N_FILS 10
#endif
*/
int main(){
  pid_t fils;
  int i;
  
  for (i=0; i < N_FILS; i++){
    if ((fils = fork()) == -1){
      
      perror("Erreur lors de la creation du fils\n"); exit (1);
      
    }else{
      
      if (fils == 0){
	
	printf("Je suis le fils %d et mon pid est : %d \n", i, getpid());
	exit(2);
	
      }else{
	/*if(i==(N_FILS-1)){*/
	
	printf("C est moi le pere mon pid : %d  \n",getpid());
	  /*}*/
      }
    }
  }
  
  return 0;
}

