/* ficindex_enonce_tests.c */
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


/* ouvre le fichier filename en lecture puis */
/* affiche son contenu */
int print_content(char * filename, int size_index){
  int des,n;
  UnIndex * buf = NULL;
  
  /* ouvrir le fichier en lecture */
  if ( (des = indopen(filename, O_RDONLY , size_index,S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen");
    return -1;
  }
  
  if ((buf = malloc(size_index)) == NULL){
    perror("malloc");
    return -1;
  }
  printf("-- debut du fichier --\n");
  /* tant que y a qqc a afficher on l'affiche */
  while ((n = indread(des, buf, size_index)) > 0){
    printf("--------\n");
    printf("\tnom: %s\n",buf[0].nom);
    printf("\tprenom: %s\n",buf[0].prenom);
    printf("\tage: %d\n",buf[0].age);
  }
  if (n == 0 ){
    printf("-- fin du fichier --\n");
    indclose(des);
    return 1;
  }else{
    perror("indread");
    return -1;
  }
}

int main(){
  int des1,n;
  UnIndex index1 = {"toto","tux",17};
  UnIndex buf[TAILLE_BUF];
  UnIndex *enreg = NULL; 
  
  
  /* ouvrir le fichier en ecriture */
  printf("+ ouvrir le fichier en ecriture \n");
  if ( (des1 = indopen("fic1", O_RDWR|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }
  

  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
    return 1;
  }

  
  printf("\n====== TEST DE indwrite ======\n");
  /* ajouter cette fiche dans le fichier indexe */
  printf("+ ajouter une fiche au fichier indexe\n");
  buf[0] = index1;
  if (indwrite(des1, buf, sizeof(UnIndex)) == -1){
    perror("indwrite\n");
    return 1;
  }
  
  
  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
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
  

  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
    return 1;
  }

  
  printf("\n====== TEST DE indclose ======\n");
  /* fermer le fichier puis l'ouvrir en mode lecture */
  printf("+ fermer le fichier puis l'ouvrir en mode lecture \n");
  indclose(des1);
  if ( (des1 = indopen("fic1", O_RDONLY|O_CREAT , sizeof(UnIndex),S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH )) == -1 ){
    perror("indopen\n");
    return 1;
  }

  
  printf("\n====== TEST DE indread ======\n");
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
  
  
  printf("\n====== TEST DE indlseek ======\n");
  /* deplacer le curseur un cran vers l'arriere */
  printf("+ deplacer le curseur un cran vers l'arriere \n");
  if ((indlseek(des1, -1, SEEK_CUR)) == -1){
    perror("indlseek\n");
    return 1;
  }
  
  /* faire une lecture: la fiche qui sera lue est */
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
  
  
  printf("\n====== TEST DE indsearch ======\n");
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

  
  printf("\n====== TEST DE indxchg ======\n");
  printf("+ changer la taille de l'index du fichier \"fic1\". Recopier les donnees de ce fichier dans le nouveau fichier \"newfic1\" \n");
  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
    return 1;
  }
  printf("+ changer la taille d'index du fichier \"fic1\". La nouvelle taille est egale a l'ancienne taille\n");
  if (indxchg("fic1", "fic2", sizeof(UnIndex) ) == -1){
    perror("indxchg");
    return 1;
  }
  printf("+ afficher le contenu du fichier \"fic2\"\n");
  if (print_content("fic2",sizeof(UnIndex)) == -1){
    return 1;
  }
 
  
  printf("+ changer la taille de l'index du fichier \"fic1\". Recopier les donnees de ce fichier dans le nouveau fichier \"newfic1\" \n");
  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
    return 1;
  }
  printf("+ changer la taille d'index du fichier \"fic1\". La nouvelle taille est SUPERIEURE a l'ancienne taille\n");
  if (indxchg("fic1", "fic_sup", 2*sizeof(UnIndex) ) == -1){
    perror("indxchg");
    return 1;
  }
  printf("+ afficher le contenu du fichier \"fic_sup\"\n");
  if (print_content("fic_sup",2*sizeof(UnIndex)) == -1){
    return 1;
  }
 

  
  printf("+ changer la taille de l'index du fichier \"fic1\". Recopier les donnees de ce fichier dans le nouveau fichier \"newfic1\" \n");
  printf("+ afficher le contenu du fichier \"fic1\"\n");
  if (print_content("fic1",sizeof(UnIndex)) == -1){
    return 1;
  }
  printf("+ changer la taille d'index du fichier \"fic1\". La nouvelle taille est INFERIEUR a l'ancienne taille\n");
  if (indxchg("fic1", "fic_inf", sizeof(UnIndex) /2 ) == -1){
    perror("indxchg");
    return 1;
  }
  printf("+ afficher le contenu du fichier \"fic_inf\"\n");
  if (print_content("fic_inf",sizeof(UnIndex)/2) == -1){
    return 1;
  }
  
  indclose(des1);
  return 0;
}
