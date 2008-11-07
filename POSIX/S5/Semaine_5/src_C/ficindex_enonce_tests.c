
#define _POSIX_SOURCE 1

#include "ficindex_enonce.h"
#define TAILLE_BUF 20


typedef struct unindex {
  char nom[NAME_MAX];
  char prenom[NAME_MAX];
  unsigned short int age;
} UnIndex;

int main(){
  int des1,n;
  UnIndex index1 = {"toto","tux",17};
  UnIndex buf[TAILLE_BUF];
  
  /* ouvrir le fichier en ecriture */
  if ( (des1 = indopen("fic1", O_WRONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }

  
  /* ajouter cette fiche dans le fichier indexe */
  buf[0] = index1;
  if (indwrite(des1, buf, sizeof(UnIndex)) == -1){
    perror("indwrite\n");
    return 1;
  }
  
  /* fermer le fichier puis l'ouvrir en mode lecture */
  indclose(des1);
  if ( (des1 = indopen("fic1", O_RDONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }

  /* lire ce que nous venons d'ecrire */
  /* a savoir la struct index1        */
  if ( (n = indread(des1, buf, sizeof(UnIndex))) == -1){
    perror("indread");
    return 1;
  }
  /* afficher la stuct recuperee du fichier indexe */
  printf("index1: \n");
  printf("\tnom: %s\n",buf[0].nom);
  printf("\tprenom: %s\n",buf[0].prenom);
  printf("\tage: %d\n",buf[0].age);
  
  return 0;
}
