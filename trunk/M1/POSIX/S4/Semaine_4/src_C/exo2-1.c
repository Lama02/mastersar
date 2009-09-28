/* exo2-1.c */
#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#define SIZE_TAMPON 100
/*
 * Retourne la position du deuxime mot si la chaine contient deux mots, -1 sinon.
 * Le premier caractere est a la position 0
 */
int contient_deux_mots(char * chaine){
  /* TODO */
  int i=0;
  while(chaine[i] != '\0'){
    if (chaine[i] == ' '){
      if (chaine[i+1] != '\0') return i+1;
    }
    i++;
  }
  return -1;
}


int position_deuxieme_mot(int desc){
  char buf[SIZE_TAMPON];
  int size;
  int i=0;
  
  if ((size = read(desc, buf, SIZE_TAMPON)) <= 0){
    fprintf(stderr,"Erreur: read\n");
    return EXIT_FAILURE;
  }
  while (buf[i] != '\0'){
    if (buf[i] == ' '){
      if (buf[i+1] != '\0') return i+1;
    }
    i++;
  }
  return -1;
}


int main(int argc, char * argv[]){

  int desc; /* le descripteur du fichier a creer */
  int taille; /* contiendra la taille de la chaine a ecrire dans le fichier */
  int position; /* la position du deuxieme mot de la chaine */

  if (argc != 4){
    fprintf(stderr, "Erreur. Nombre d'argument invalide.\n");
    return 1;
  }else{
    /* Creer le fichier passe en parametre */
    if ( (desc = creat(argv[1],S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)) == -1){
      fprintf(stderr,"Erreur: creat\n");
      return EXIT_FAILURE;
    }else{
      /*
       * mettre la chaine de caractere dans le fichier
       * qu'on vient de cree
       */
      taille = strlen(argv[2]);
      if (write(desc,argv[2],taille) == -1){
	fprintf(stderr,"Erreur: write\n");
	return EXIT_FAILURE;
      }else{
	/*
	 * la chaine est ecrite dans le fichier
	 * on peut fermer le fichier
	 */
	if (close(desc) == -1){
	  fprintf(stderr,"Erreur: close\n");
	  return EXIT_FAILURE;
	}else{
	  /*
	   * nous allons reouvrir le fichier a nouveau
	   */
	  if (( desc = open(argv[1],O_RDWR)) == -1){
	    fprintf(stderr,"Erreur: open\n");
	    return EXIT_FAILURE;
	  }else{
	    /* le fichier est ouvert en lecture-ecriture */
	    /* on se deplace jusqu'au deuxieme mot */
	    if ( (position = position_deuxieme_mot(desc)) == -1){
	      fprintf(stderr,"Erreur: pas de deuxieme mot dans la premiere chaine\n");
	      return EXIT_FAILURE;
	    }
	    if (lseek(desc, position, SEEK_SET) == -1){
	      fprintf(stderr,"Erreur: lseek\n");
	      return EXIT_FAILURE;
	    }else{
	      /* nous allons ecraser le deuxieme mot par
	       * argv[3] la deuxieme chaine passe en parametre
	       */
	      taille = strlen(argv[3]);
	      if (write(desc,argv[3],taille) == -1){
		fprintf(stderr,"Erreur: write\n");
		return EXIT_FAILURE;
	      }else{
		/* supprimer le reste du fichier*/
		/* marche pas
		   truncate(desc, lseek(desc,0,SEEK_CUR));
		*/
		if (close(desc) == -1){
		  fprintf(stderr,"Erreur: close\n");
		  return EXIT_FAILURE;
		}
		return 0;
	      }
	    }
	  }
	}
      }
    }
  }
}
