/* Programme de multiplication de deux matrices carrees  */
/* dans le main vous trouverez deux jeux de test. */


#include<stdio.h>
#include <stdlib.h>

#include <sys/time.h>

double my_gettimeofday(){
  struct timeval tmp_time;
  gettimeofday(&tmp_time, NULL);
  return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}





void matmul(int n, double *a, double *b, double *c)
{
  int i,j,k;
  double s;

  for(i=0; i<n ; i++){
    for(j=0; j<n ; j++){
      s=0;
      for(k=0; k<n ; k++)
	s+=a[i*n+k]*b[k*n+j];
      c[i*n+j]=s;
    }
  }
}


/* programme de test */

int main()
{
  int i,j,k,n;
  double t_start = 0.0, t_stop = 0.0;


  /******************************************************/
  /******************************************************/
  /* jeu de test 1*/
  
  /*   n=2; */
  /*   double a[]={1,2,2,1}, b[]={1,1,1,2},c[n*n]; */
  
  /* résultat sous forme matriciel */
  /*     c=  */
  /*     +3.000000e+00  +5.000000e+00 */
  /*     +3.000000e+00  +4.000000e+00 */



  /******************************************************/
  /******************************************************/
  /* jeu de test 2 */
  
      n=800;
      double *a,*b,*c;
      if (((a=malloc(n*n*sizeof(double)))==NULL)||
          ((b=malloc(n*n*sizeof(double)))==NULL)||
          ((c=malloc(n*n*sizeof(double)))==NULL)){
        fprintf(stderr,"erreur allocation memoire\n");
        exit(1);
      }
  
      for(i=0; i<n ; i++)
        for(j=0; j<n ; j++){
          a[i*n+j]=1/(double)(i+j+1);
          b[j*n+j]=a[i*n+j];
        }
  
  /*   n=800 premier carré :  */
  /*   +1.250000e-03  +6.242197e-04 */
  /*   +6.250000e-04  +4.161465e-04 */
  
  /*   n=800 deuxième carré: */
  /*   +3.918486e-07  +3.913585e-07 */
  /*   +3.916034e-07  +3.911137e-07 */
  


  /******************************************************/
  /******************************************************/
  
  t_start=my_gettimeofday();
  
  matmul(n,a,b,c);
  
  t_stop=my_gettimeofday();


  /* affichage du premier carre 2x2 de la matrice c*/
  for(i=0; i<2 ; i++){
    for(j=0; j<2 ; j++)
      printf("%+e  ",c[i*n+j]);
    printf("\n");
  }
  printf("\n");
  /* affichage du dernier carre 2x2 de la matrice c*/
  for(i=n-2; i<n ; i++){
    for(j=n-2; j<n ; j++)
      printf("%+e  ",c[i*n+j]);
    printf("\n");
  }

  printf("\nTemps de calcul : %gs\n", t_stop - t_start);
}
 

