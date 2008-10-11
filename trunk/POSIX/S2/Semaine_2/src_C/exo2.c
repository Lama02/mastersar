#define _PSOX_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

#ifndef N
#define N 10
#endif

int main(){

  int i=0,status;
  pid_t fils,tmp;

  while(i<N && (fils=fork())==0){

    i++;
    printf("je suis le fils : %d\n\n",getpid());
}

  if(i>N || i==N){
    exit(i);

      }else{

    tmp=wait(&status);
    if(i==0){
      printf("PERE:le fils %d termine, status : %d \n",tmp,WEXITSTATUS(status)); 
    }
}

  return 0;

}
