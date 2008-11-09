
#define _POSIX_SOURCE 1

#include "ficindex_enonce.h"
#define TAILLE_BUF 20


typedef struct unindex {
  char nom[NAME_MAX];
  char prenom[NAME_MAX];
  unsigned short int age;
} UnIndex;

/* definir notre fonction de comparaison */
int nomstrcmp(void *ui, void *ch) { 
  return (strcmp(((UnIndex *)ui)->nom, (char *)ch)); 
} 


int main(){
  int des1,n;
  UnIndex index1 = {"toto","tux",17};
  UnIndex buf[TAILLE_BUF];
  UnIndex *enreg = NULL; 

  
  /* ouvrir le fichier en ecriture */
  printf("+ ouvrir le fichier en ecriture \n");
  if ( (des1 = indopen("fic1", O_WRONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }

  
  /* ajouter cette fiche dans le fichier indexe */
  printf("+ ajouter une fiche au fichier indexe\n");
  buf[0] = index1;
  if (indwrite(des1, buf, sizeof(UnIndex)) == -1){
    perror("indwrite\n");
    return 1;
  }

  
  /* ajouter cette fiche dans le fichier indexe */
  printf("+ ajouter une autre fiche au fichier indexe\n");
  strcpy(index1.nom, "tutux") ;
  strcpy(index1.prenom, "tomtom") ;
  index1.age = 24;
  buf[0] = index1;
  if (indwrite(des1, buf, sizeof(UnIndex)) == -1){
    perror("indwrite\n");
    return 1;
  }
   
 
  /* fermer le fichier puis l'ouvrir en mode lecture */
  printf("+ fermer le fichier puis l'ouvrir en mode lecture \n");
  indclose(des1);
  if ( (des1 = indopen("fic1", O_RDONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }


  /* lire ce que nous venons d'ecrire */
  /* a savoir les deux fiches         */
  printf("+ lire les deux fiches que nous venons d'ajouter\n");
  /* la premiere */
  if ( (n = indread(des1, buf, sizeof(UnIndex))) == -1){
    perror("indread");
    return 1;
  }
  /* la deuxieme */
  if ( (n = indread(des1, &buf[1], sizeof(UnIndex))) == -1){
    perror("indread");
    return 1;
  }
  
  
  /* afficher les deux fiches recuperees du fichier indexe */
  printf("+ afficher les deux fiches lues\n");
  /* la premiere */
  printf("index1: \n");
  printf("\tnom: %s\n",buf[0].nom);
  printf("\tprenom: %s\n",buf[0].prenom);
  printf("\tage: %d\n",buf[0].age);
  /* la deuxieme */
  printf("index1: \n");
  printf("\tnom: %s\n",buf[1].nom);
  printf("\tprenom: %s\n",buf[1].prenom);
  printf("\tage: %d\n",buf[1].age);
  
  
  /* deplacer le curseur un cran vers l'arriere */
  printf("+ deplacer le curseur un cran vers l'arriere \n");
  if ((indlseek(des1, -1, SEEK_CUR)) == -1){
    perror("indlseek\n");
    return 1;
  }
  
  
  /* faire un lecture: la fiche qui sera lue est */
  /* mormalement la deuxieme */
  printf("+ faire une lecture. La deuxieme fiche sera lue\n");
  if ( (n = indread(des1, buf, sizeof(UnIndex))) == -1){
    perror("indread");
    return 1;
  }

  
  /* afficher la fiche lue */
  printf("+ afficher la fiche lue \n");
  printf("index1: \n");
  printf("\tnom: %s\n",buf[0].nom);
  printf("\tprenom: %s\n",buf[0].prenom);
  printf("\tage: %d\n",buf[0].age);

  /* se deplacer au debut du fichier */
  printf("+ se deplacer au debut du fichier\n");
  if (indlseek(des1, 0, SEEK_SET) == -1){
    perror("indlseek");
    return 1;
  }
  /* faire une lecture */
  printf("+ faire une lecture.\n");
  if ( (n = indread(des1, buf, sizeof(UnIndex))) == -1){
    perror("indread");
    return 1;
  }
  printf("+ afficher la fiche lue \n");
  printf("index1: \n");
  printf("\tnom: %s\n",buf[0].nom);
  printf("\tprenom: %s\n",buf[0].prenom);
  printf("\tage: %d\n",buf[0].age);
  
  /* trouver la fiche correspondant a la cle passee en arg */
  printf("+ recherche de la fiche correspondant au nom \"dupont\".\n");
  enreg=indsearch(des1, nomstrcmp, "dupont");
  if (enreg == NULL){
    printf("\taucune fiche avec \"dupont\" comme nom\n");
  }else{
    printf("\ton doit pas voir ce message\n");
  }
  
  printf("+ recherche de la fiche correspondant au nom \"tutux\".\n");
  enreg=indsearch(des1, nomstrcmp, "tutux");
  if (enreg == NULL){
    printf("\ton doit pas voir ce message\n");
  }else{
    printf("\t- la fiche suivante match:\n");
    printf("\t\t nom: %s\n",enreg->nom);
    printf("\t\t prenom: %s\n",enreg->prenom);
    printf("\t\t age: %d\n",enreg->age);
  }
  
  
  return 0;
}
