/* exo1-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char * argv[]){
  DIR * pt_Dir;
  struct dirent * dirEnt;
  char *buf; /* contiendra le nom du rep courant*/
  extern int errno;
  
  if (argc > 2){
    fprintf(stderr, "Erreur. Nombre d'argument invalide.\n");
    return 1;
  }
  
  if (argc < 2){
    /* 
     * pas d argument 
     * on liste le contenu du repertoire courant 
     */
    
    /*
     * on recupere le chemin vers le repertoire courant 
     */
    buf = malloc(PATH_MAX);
    if (( getcwd(buf, PATH_MAX)) == NULL){
      fprintf(stderr,"Erreur : getcwd\n");
    }
    printf("buf = %s\n",buf);
    if ((pt_Dir = opendir(buf))==NULL){
      fprintf(stderr,"Erreur: opendir\n");
      return EXIT_FAILURE;
    }
    while (( dirEnt=readdir(pt_Dir) ) != NULL){
      printf("%s\n",dirEnt->d_name);
    }
    return 0;
  }else{
    /*
      on verifie si le repertoire existe
    */
    if ((pt_Dir = opendir(argv[1])) == NULL){
      if (errno == ENOENT){
	/* le repertoire n existe pas */
	/* nous allons le creer */
	if (!mkdir(argv[1], S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)){
	  return 0;
	}else{
	  fprintf(stderr,"Erreur: mkdir\n");
	  return EXIT_FAILURE;
	}
	      
      }else{
	/* autres erreurs */
	fprintf(stderr,"Erreur: opendir\n");
	return EXIT_FAILURE;
      }
    }else{
      /* le rep existe nous allons afficher sont contenu */
      
      return 0;
    }
  } 
}

  
