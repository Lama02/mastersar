#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>

#define SIZE_H_N 50

int question1_2(int argc, char* argv[])
{
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[100];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);

  if (my_rank != 0)
    {
      sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);
      dest = 0;
      MPI_Send(message, 
	       strlen(message)+1, 
	       MPI_CHAR, 
	       dest, 
	       tag, 
	       MPI_COMM_WORLD);
    }else{
    for(source = 1; source < p; source++)
      {
	MPI_Recv(message,
		 100,
		 MPI_CHAR, 
		 source,
		 tag,
		 MPI_COMM_WORLD, 
		 &status);
	printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);
      }
  }
  MPI_Finalize();
}
int question3(int argc, char* argv[])
{
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[100];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);

  if (my_rank != 0)
    {
      sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);
      dest = 0;
      MPI_Send(message, 
	       strlen(message)+1, 
	       MPI_CHAR, 
	       dest, 
	       tag, 
	       MPI_COMM_WORLD);
    }else{
    for(source = 1; source < p; source++)
      {
	MPI_Recv(message,
		 100,
		 MPI_CHAR, 
		 MPI_ANY_SOURCE,
		 tag,
		 MPI_COMM_WORLD, 
		 &status);
	printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);
      }
  }
  MPI_Finalize();
}
int question4partie1(int argc, char* argv[])
{
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[100];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);


  sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);

  if (my_rank==p-1)
    {
      dest = 0;
    }else{
    dest = my_rank+1;
  }
  MPI_Send(message, 
	   strlen(message)+1, 
	   MPI_CHAR, 
	   dest, 
	   tag, 
	   MPI_COMM_WORLD);

  if (my_rank==0)
    {
      source = p-1;
    }else{
    source = my_rank-1;
  }
  MPI_Recv(message,
	   100,
	   MPI_CHAR, 
	   source,
	   tag,
	   MPI_COMM_WORLD, 
	   &status);
  printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);


  MPI_Finalize();
}

int question4partie2(int argc, char* argv[])
{
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[100];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);


  sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);

  if (my_rank==p-1)
    {
      dest = 0;
    }else{
    dest = my_rank+1;
  }
  MPI_Ssend(message, 
	    strlen(message)+1, 
	    MPI_CHAR, 
	    dest, 
	    tag, 
	    MPI_COMM_WORLD);

  if (my_rank==0)
    {
      source = p-1;
    }else{
    source = my_rank-1;
  }
  MPI_Recv(message,
	   100,
	   MPI_CHAR, 
	   source,
	   tag,
	   MPI_COMM_WORLD, 
	   &status);
  printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);


  MPI_Finalize();
}

int question5(int argc, char* argv[])
{
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[100];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);




  if (my_rank==0)
    {
      sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);
      MPI_Ssend(message, 
		strlen(message)+1, 
		MPI_CHAR, 
		1, 
		tag, 
		MPI_COMM_WORLD);
      MPI_Recv(message,
	       100,
	       MPI_CHAR, 
	       p-1,
	       tag,
	       MPI_COMM_WORLD, 
	       &status);
      printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);   
    }else{
    MPI_Recv(message,
	     100,
	     MPI_CHAR, 
	     my_rank-1,
	     tag,
	     MPI_COMM_WORLD, 
	     &status);
    printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);   
    sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);
    if (my_rank == p-1)
      {
	dest=0;
      }else{
      dest=my_rank+1;
    }
    MPI_Ssend(message, 
	      strlen(message)+1, 
	      MPI_CHAR, 
	      dest, 
	      tag, 
	      MPI_COMM_WORLD);
     

  }

  


  MPI_Finalize();
}
int question6(int argc, char* argv[],int taille)
{
  int size = taille;
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[size];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);


  sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);

  if (my_rank==p-1)
    {
      dest = 0;
    }else{
    dest = my_rank+1;
  }
  MPI_Send(message, 
	   size, 
	   MPI_CHAR, 
	   dest, 
	   tag, 
	   MPI_COMM_WORLD);
  
  if (my_rank==0)
    {
      source = p-1;
    }else{
    source = my_rank-1;
  }
  MPI_Recv(message,
	   size,
	   MPI_CHAR, 
	   source,
	   tag,
	   MPI_COMM_WORLD, 
	   &status);
  printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);


  MPI_Finalize();
}

int question7(int argc, char* argv[],int taille)
{
  int size = taille;
  int my_rank;
  int p;
  int source;
  int dest;
  int tag = 0;
  char message[size];
  MPI_Status status;
  char hostname[SIZE_H_N];

  gethostname(hostname, SIZE_H_N);

  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);


  sprintf(message, "Coucou du processus #%d depuis %s!", my_rank, hostname);

  if (my_rank==p-1)
    {
      dest = 0;
    }else{
    dest = my_rank+1;
  }
  MPI_Send(message, 
	   size, 
	   MPI_CHAR, 
	   dest, 
	   tag, 
	   MPI_COMM_WORLD);
  
  if (my_rank==0)
    {
      source = p-1;
    }else{
    source = my_rank-1;
  }
  MPI_Recv(message,
	   size,
	   MPI_CHAR, 
	   source,
	   tag,
	   MPI_COMM_WORLD, 
	   &status);
  printf("Sur %s, le processus #%d a recu le message : %s \n", hostname, my_rank, message);


  MPI_Finalize();
}

int main (int argc, char* argv[]){
  printf("Coucou\n");
  question1_2(argc, argv);

  //question3(argc, argv);

  //question4partie1(argc, argv);

  /* ne marche pas car l'execution de cette fonction provoque un interblocage car :
     chaque process attend la reception de son message avant de se mettre en reception */
  //question4partie2(argc, argv);

  //question5(argc, argv);
  int taille = 256*255;
  //question6(argc, argv,taille);
  //  question7(argc, argv);
  return 0;
}
