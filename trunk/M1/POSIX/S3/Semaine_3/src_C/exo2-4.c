#include <stdio.h>

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1

int main (int argc, char * argv[]){
  
  int i;

  if (fork()==0){
    /* je suis dans le fils */
    printf("Execution du fils.\n");
    /* sleep (3); */
  }else{
    /* je suis dans le pere */
    printf("Execution du pere.\n");
    
  }
    
  

  return EXIT_SUCCESS;
}
