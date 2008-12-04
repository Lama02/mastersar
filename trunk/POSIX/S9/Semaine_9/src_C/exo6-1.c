/* exo6-1.c*/
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"
#include <pthread.h>



void* func_thread (void* arg){
  FILE* fp1, *fp2;
  int c;
  c = 0;
  fp1= fopen ((char*) arg, "r");
  fp2= fopen ((char*) arg, "r+");
  
  if ((fp1== NULL) || (fp2== NULL)) {
    perror("fopen");
    exit (1);
  }
  
  while (c !=EOF) {
    c=fgetc(fp1);
    
    if (c!=EOF)
      fputc(toupper(c),fp2);
  }
  
  fclose (fp1);
  fclose (fp2);
  
  pthread_exit((void *) 0 );
  
}



int main (int argc, char ** argv){

  int i, p, status;
    
  int  NB_THREAD = argc - 1; 

  /* tableau dans lequel on recupere les id des threads*/
  pthread_t tid[NB_THREAD];

  for(i=0; i<NB_THREAD; i++){
    if((p=pthread_create(&(tid[i]), NULL, func_thread, (void*)argv[i+1] )) != 0){
      perror("pthread_create \n");
      exit(1);
    }
  }
  
   /* attendre la fin de toutes les threads creees */
  for(i=0; i<NB_THREAD; i++){
   if(pthread_join(tid[i],(void**) &status) != 0){
      perror("pthread_join"); 
      exit(1);
    }
  }

  
  return EXIT_SUCCESS;
  
}
