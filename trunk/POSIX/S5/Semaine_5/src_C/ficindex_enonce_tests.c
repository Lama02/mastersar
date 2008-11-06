
#define _POSIX_SOURCE 1

#include "ficindex_enonce.h"



typedef struct unindex {
  char nom[NAME_MAX];
  char prenom[NAME_MAX];
  unsigned short int age;
} UnIndex;

int main(){
  int des1;
  UnIndex index1;

  /* ouvrir le fichier en ecriture */
  if ( (des1 = indopen("fic1", O_WRONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }

  index1.nom = "toto";
  index1.prenom = "tux";
  index1.age = 17;
  
  /* ajouter cette fiche dans le fichier indexe */
    
  
  return 0;
}
