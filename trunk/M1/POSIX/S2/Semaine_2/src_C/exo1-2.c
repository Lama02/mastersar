/**** decoder.c ****/

#define _POSIX_SOURCE 1


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int magicsq[3][3] = {{2, -1, 4}, {-1, -1, -1}, {6, -1, 8}};

int check() {
  int i, j,result, sums[8];
  for (i = 0; i < 8; i++) 
    sums[i] = 0;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      sums[i] += magicsq[i][j];
      sums[i+3] += magicsq[j][i];
      if (i == j) sums[6] += magicsq[i][j];
      if ((i+j) == 2) sums[7] += magicsq[i][j];
    }
  }
  /*  int result = 1; */
  result = 1;
  i = 1;
  while ((i < 8) && (result == 1)) {
    if (sums[0] != sums[i])
      result = 0;
    i++;
  }
  return result;
}


void display() {
  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++)
      printf("  %d", magicsq[i][j]);
    printf("\n");
  }
  printf("\n");
}


int solve(int x) {
  /* la fonction retourne 1 si la matrice est magique, sinon 0 */
  
  /* CODE A EXECUTER PAR CHAQUE PROCESSUS FILS */
  

  /* le numero magique */
  int num = x + magicsq[0][0] + magicsq[0][2];
  int b, c, d, e, res;
  
  
  b = num - magicsq[0][0] - magicsq[2][0];
  d = num - magicsq[0][2] - magicsq[2][2];
  e = num - magicsq[2][0] - magicsq[2][2];
  c = num - b - d;
  
  magicsq[0][1] = x;
  magicsq[1][0] = b;
  magicsq[1][1] = c;
  magicsq[1][2] = d;
  magicsq[2][1] = e;
  
  res = check();
  
  return res;
}

int main(int argc, char **argv)
{
  /* LANCEMENT ET ATTENTE DES PROCESSUS FILS */
  
  pid_t fils;
  int i=0;
  
  
  while (i < 10){
    if ( (fils=fork()) == 0 ){
      /*si je suis dans le fils */
      if (solve(i)) {
	display() ;
      }
      exit(0);
    }else{
      /*si je suis dans le pere */
      i++;
    }
  }
  
  /* on a attend la mort des tous les fils */
  i=0;
  while(i<10){
    wait(NULL);
    i++;
  }
  
  return 0;
}
