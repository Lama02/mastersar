
/*
 * @UFR@
 * @MODULE@
 * Université Pierre et Marie Curie
 * Calcul de convolution sur une image.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>   /* pour le rint */
#include <string.h> /* pour le memcpy */
#include <time.h>   /* chronometrage */
#include <mpi.h>

#include "rasterfile.h"

#define MAX(a,b) ((a>b) ? a : b)

double my_gettimeofday(){
  struct timeval tmp_time;
  gettimeofday(&tmp_time, NULL);
  return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}


/** 
 * \struct Raster
 * Structure décrivant une image au format Sun Raster
 */

typedef struct {
  struct rasterfile ras;  ///< Entête image Sun Raster
  unsigned char rouge[256],vert[256],bleu[256];  ///< Palette de couleur
  unsigned char *data;    ///< Pointeur vers l'image
} Raster;


/**
 * Cette procedure convertit un entier LINUX en un entier SUN 
 *
 * \param i pointeur vers l'entier à convertir
 */

void swap(int *i) {
  unsigned char s[4],*n;
  memcpy(s,i,4);
  n=(unsigned char *)i;
  n[0]=s[3];
  n[1]=s[2];
  n[2]=s[1];
  n[3]=s[0];
}

/**
 * \brief Lecture d'une image au format Sun RASTERFILE.
 *
 * Au retour de cette fonction, la structure r est remplie
 * avec les données liée à l'image. Le champ r.ras contient
 * les informations de l'entete de l'image (dimension, codage, etc).
 * Le champ r.data est un pointeur, alloué par la fonction
 * lire_rasterfile() et qui contient l'image. Cette espace doit
 * être libéré après usage.
 *
 * \param nom nom du fichier image
 * \param r structure Raster qui contient l'image
 *  chargée en mémoire
 */

void lire_rasterfile(char *nom, Raster *r) {
  FILE *f;
  struct rasterfile ras;
  int i;
    
  if( (f=fopen( nom, "r"))==NULL) {
    fprintf(stderr,"erreur a la lecture du fichier %s\n", nom);
    exit(1);
  }
  fread( &(r->ras), sizeof(struct rasterfile), 1, f);    
  swap(&(r->ras.ras_magic));
  swap(&(r->ras.ras_width));
  swap(&(r->ras.ras_height));
  swap(&(r->ras.ras_depth));
  swap(&(r->ras.ras_length));
  swap(&(r->ras.ras_type));
  swap(&(r->ras.ras_maptype));
  swap(&(r->ras.ras_maplength));
    
  if ((r->ras.ras_depth != 8) ||  (r->ras.ras_type != RT_STANDARD) ||
      (r->ras.ras_maptype != RMT_EQUAL_RGB)) {
    fprintf(stderr,"palette non adaptee\n");
    exit(1);
  }
    
  /* composante de la palette */
  fread(&(r->rouge),r->ras.ras_maplength/3,1,f);
  fread(&(r->vert), r->ras.ras_maplength/3,1,f);
  fread(&(r->bleu), r->ras.ras_maplength/3,1,f);
    
  if ((r->data=malloc(r->ras.ras_width*r->ras.ras_height))==NULL){
    fprintf(stderr,"erreur allocation memoire\n");
    exit(1);
  }
  fread(r->data,r->ras.ras_width*r->ras.ras_height,1,f);
  fclose(f);
}

/**
 * Sauve une image au format Sun Rasterfile
 */

void sauve_rasterfile(char *nom, Raster *r)     {
  FILE *f;
  struct rasterfile ras;
  int i;
  
  if( (f=fopen( nom, "w"))==NULL) {
    fprintf(stderr,"erreur a l'ecriture du fichier %s\n", nom);
    exit(1);
  }
    
  swap(&(r->ras.ras_magic));
  swap(&(r->ras.ras_width));
  swap(&(r->ras.ras_height));
  swap(&(r->ras.ras_depth));
  swap(&(r->ras.ras_length));
  swap(&(r->ras.ras_type));
  swap(&(r->ras.ras_maptype));
  swap(&(r->ras.ras_maplength));
    
  fwrite(&(r->ras),sizeof(struct rasterfile),1,f);
  /* composante de la palette */
  fwrite(&(r->rouge),256,1,f);
  fwrite(&(r->vert),256,1,f);
  fwrite(&(r->bleu),256,1,f);
  /* pour le reconvertir pour la taille de l'image */
  swap(&(r->ras.ras_width));
  swap(&(r->ras.ras_height));
  fwrite(r->data,r->ras.ras_width*r->ras.ras_height,1,f); 
  fclose(f);
}

/**
 * Réalise une division d'entiers plus précise que
 * l'opérateur '/'.
 * Remarque : la fonction rint provient de la librairie 
 * mathématique.
 */

unsigned char division(int numerateur,int denominateur) {
  
  if (denominateur != 0)
    return (unsigned char) rint((double)numerateur/(double)denominateur); 
  else 
    return 0;
}

static int ordre( unsigned char *a, unsigned char *b) {
  return (*a-*b);
}


typedef enum {
  CONVOL_MOYENNE1, ///< Filtre moyenneur
  CONVOL_MOYENNE2, ///< Filtre moyenneur central
  CONVOL_CONTOUR1, ///< Laplacien
  CONVOL_CONTOUR2, ///< Max gradient
  CONVOL_MEDIAN    ///< Filtre médian
} filtre_t;

/**
 * Réalise une opération de convolution avec un noyau prédéfini sur
 * un point.
 *
 * \param choix type de noyau pour la convolution :
 *  - CONVOL_MOYENNE1 : filtre moyenneur
 *  - CONVOL_MOYENNE2 : filtre moyenneur avec un poid central plus fort
 *  - CONVOL_CONTOUR1 : filtre extracteur de contours (laplacien)
 *  - CONVOL_CONTOUR2 : filtre extracteur de contours (max des composantes du gradient)
 *  - CONVOL_MEDIAN : filtre médian (les 9 valeurs sont triées et la valeur
 *     médiane est retournée).
 * \param NO,N,NE,O,CO,E,SO,S,SE: les valeurs des 9 points
 *  concernés pour le calcul de la convolution (cette dernière est
 *  formellement une combinaison linéaire de ces 9 valeurs).
 * \return la valeur de convolution.
 */

unsigned char filtre( filtre_t choix, 
		      unsigned char NO, unsigned char N,unsigned char NE, 
		      unsigned char O,unsigned char CO, unsigned char E, 
		      unsigned char SO,unsigned char S,unsigned char SE) {
  int numerateur,denominateur;

  switch (choix)
    {
    case CONVOL_MOYENNE1:
      /* filtre moyenneur */
      numerateur = (int)NO + (int)N + (int)NE + (int)O + (int)CO + 
	(int)E + (int)SO + (int)S + (int)SE;
      denominateur = 9;
      return division(numerateur,denominateur); 

    case CONVOL_MOYENNE2:
      /* filtre moyenneur */
      numerateur = (int)NO + (int)N + (int)NE + (int)O + 4*(int)CO +
	(int)E + (int)SO + (int)S + (int)SE;
      denominateur = 12;
      return division(numerateur,denominateur);	

    case CONVOL_CONTOUR1:
      /* extraction de contours */
      numerateur = -(int)N - (int)O + 4*(int)CO - (int)E - (int)S;
      /* numerateur = -(int)NO -(int)N - (int)NE - (int)O + 8*(int)CO -
	 (int)E - (int)SO - (int)S - (int)SE;
      */
      return ((4*abs(numerateur) > 255) ? 255 :  4*abs(numerateur));

    case CONVOL_CONTOUR2:
      /* extraction de contours */
      numerateur = MAX(abs(CO-E),abs(CO-S));
      return ((4*numerateur > 255) ? 255 :  4*numerateur);

    case CONVOL_MEDIAN:{
      unsigned char tab[] = {NO,N,NE,O,CO,E,SO,S,SE};
      /* filtre non lineaire : tri rapide sur la brillance */
      qsort( tab, 9, sizeof(unsigned char), (int (*) (const void *,const void *))ordre);
      return tab[4];
    }

    default:
      printf("\nERREUR : Filtre inconnu !\n\n");
      exit(1);
    }
}

/**
 * Convolution d'une image par un filtre prédéfini
 * \param choix choix du filtre (voir la fonction filtre())
 * \param tab pointeur vers l'image
 * \param nbl, nbc dimension de l'image
 *
 * \sa filtre()
 */

int convolution( filtre_t choix, unsigned char tab[],int nbl,int nbc) {
  int i,j;
  unsigned char *tmp;
  
  /* Allocation memoire du tampon intermediaire : */
  tmp = (unsigned char*) malloc(sizeof(unsigned char) *nbc*nbl);
  if (tmp == NULL) {
    printf("Erreur dans l'allocation de tmp dans convolution \n");
    return 1;
  }
  
  /* on laisse tomber les bords */
  for(i=1 ; i<nbl-1 ; i++){
    for(j=1 ; j<nbc-1 ; j++){
      tmp[i*nbc+j] = filtre(
			    choix,
			    tab[(i+1)*nbc+j-1],tab[(i+1)*nbc+j],tab[(i+1)*nbc+j+1],
			    tab[(i  )*nbc+j-1],tab[(i)*nbc+j],tab[(i)*nbc+j+1],
			    tab[(i-1)*nbc+j-1],tab[(i-1)*nbc+j],tab[(i-1)*nbc+j+1]);
    } /* for j */
  } /* for i */
  
  /* Recopie de l'image apres traitement dans l'image initiale,
   * On remarquera que la premiere, la derniere ligne, la premiere
   * et la derniere colonne ne sont pas copiées (ce qui force a faire
   * la copie ligne par ligne). */
  for( i=1; i<nbl-1; i++){
    memcpy( tab+nbc*i+1, tmp+nbc*i+1, (nbc-2)*sizeof(unsigned char));
  } /* for i */
  
  /* Liberation memoire du tampon intermediaire : */
  free(tmp);   
}


/**
 * Interface utilisateur
 */

static char usage [] = "Usage : %s <nom image SunRaster> [0|1|2|3|4] <nbiter>\n";

/*
 * Partie principale
 */

int main(int argc, char *argv[]) {

  /* Variables se rapportant a l'image elle-meme */
  Raster r;
  int    w, h;	/* nombre de lignes et de colonnes de l'image */
  
  /* Variables liees au traitement de l'image */
  int 	 filtre;		/* numero du filtre */
  int 	 nbiter;		/* nombre d'iterations */
  
  /* Variables liees au chronometrage */
  double debut, fin;
  
  /* Variables de boucle */
  int 	i,j;

  /* variables MPI */
  int my_rank;        /* le rang du precess */
  int p;              /* nombre de process  */
  int source;        
  int dest;          
  int tag = 0;        
  MPI_Status status;

  /* variables de parallelisation */
  int lh;                /* Hauteur local de chaque bloc (nombre de lignes) sur lequel le proces 
			    va travailler */
  
  int l;                 /* Nombre de linges envoyé à chaque process */
  
  /**
   * buffer local a chaque process. Sert a stocker le bloc 
   * augmenter de deux linges dans le cas général et d'une 
   * seule linge dans les deux process 0 et p-1
   */
  unsigned char * tmp;   
  
  /* ligne calculé par le process. Celle qu'il doit 
   * retourné à son prédecesseur
   */
  unsigned char * premiere_ligne_calculee;   
  /*
   * La ligne recu du process predecessur
   */
  unsigned char * premiere_ligne_du_buffer;
  unsigned char * derniere_ligne_calculee;
  /*
   * La ligne recu du successeur  
   */  
  unsigned char * derniere_ligne_du_buffer;


  printf(" [DEBUG] avant init");
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);
  
  
  /* debut du chronometrage */
  debut = my_gettimeofday();            
  
  
  if (argc != 4) {
    fprintf( stderr, usage, argv[0]);
    return 1;
  }
      
  /* Saisie des paramètres */
  filtre = atoi(argv[2]);
  nbiter = atoi(argv[3]);
        
  /* Lecture du fichier Raster */
  lire_rasterfile( argv[1], &r);
  
  h = r.ras.ras_height;
  w = r.ras.ras_width;

  /* ====================================
   *           INITIALISATION 
   * ===================================
   */
  
  printf(" [DEBUG] bcast....\n");
  /*
   * Envoyer les variables de parallelisation au 
   * autres precessus
   */
  MPI_Bcast(&h,
	    sizeof(int),
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);
  MPI_Bcast(&w,
	    sizeof(int),
	    MPI_INT,
	    0,
	    MPI_COMM_WORLD);
  
  /* Calcul de la hauteur locale */
  lh = h/p+(my_rank>0?1:0)+(my_rank<p-1?1:0); 
  l = h/p;
  
  printf(" [DEBUG] malloc ....\n");
  /* 
   * allocation du buffer qui contiendra les pixels 
   * necessaire pour le calcul de la convolution 
   * sur le bloc recu
   */
  tmp = (unsigned char*) malloc(sizeof(unsigned char) *lh*w);
  if (tmp == NULL) {
    printf("Erreur dans l'allocation de tmp dans convolution \n");
    return 1;
  }
  

  printf(" [DEBUG] envoie des portions d'image ....\n");
  /* 
   * Envoyer les differents portions de l'image au differents 
   * process
   */
  if(my_rank != 0){
    /* les autres process placent le bloc recu, une ligne apres le 
     * debut de leur buffer
     */
    MPI_Scatter(r.data, 
		l,
		MPI_UNSIGNED_CHAR,
		tmp+w,              /* on saute la premiere ligne */
		l,
		MPI_UNSIGNED_CHAR,
		0,
		MPI_COMM_WORLD);
  }else{
    /* le premier precess met le bloc recu au debut de son buffer */
    MPI_Scatter(r.data, 
		l,
		MPI_UNSIGNED_CHAR,
		tmp,               /* on met au debut du buffer */
		l,
		MPI_UNSIGNED_CHAR,
		0,
		MPI_COMM_WORLD);
  }

  
  
  /* ==================================== 
   *           COMMUNICATION 
   * ====================================
   */ 
  
  printf(" [DEBUG] communication ....\n");
  
  premiere_ligne_du_buffer = tmp;
  premiere_ligne_calculee = premiere_ligne_du_buffer + w;
  /* la valeur de  lh change en fonction du process (1er, dernier ou intermediaire) */
  derniere_ligne_du_buffer = premiere_ligne_du_buffer + ((lh - 1) * w) ;
  derniere_ligne_calculee = premiere_ligne_du_buffer + ((lh - 2) * w) ;
  
  /* La convolution a proprement parler */
  for(i=0 ; i < nbiter ; i++){
    if (my_rank == 0){
      /* le premier process recevera d'abord avant d'envoyer 
       * Ceci nous permettra d'éviter un inter-blocage. 
       * Ce processur débloquera tout le monde
       */
      /* reception de la derniere ligne */
      printf(" [DEBUG] p = %d\n", p);
      printf(" [DEBUG] process -> %d \t reception de message du process %d \n", my_rank, my_rank + 1);
      MPI_Recv(derniere_ligne_du_buffer,
	       w,
	       MPI_UNSIGNED_CHAR,
	       my_rank+1,
	       0,
	       MPI_COMM_WORLD,
	       &status);
      printf(" [DEBUG] process -> %d \t reception de message du process %d ---- OK\n", my_rank, my_rank + 1);
      /*envoie de la derniere ligne calculé au process suivant*/
      printf(" [DEBUG] process -> %d \t envoie de message à process %d \n", my_rank, my_rank + 1);
      MPI_Send(derniere_ligne_calculee,
	       w,
	       MPI_UNSIGNED_CHAR,
	       my_rank+1,
	       0,
	       MPI_COMM_WORLD); 
    }else{
      if (my_rank == (p-1)){
	/* reception de la derniere ligne */
	printf(" [DEBUG] process -> %d \t send message au process %d \n", my_rank, my_rank - 1);
	MPI_Send(premiere_ligne_calculee,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank-1,
		 0,
		 MPI_COMM_WORLD);
	printf(" [DEBUG] process -> %d \t send message au process %d ------ OK\n", my_rank, my_rank - 1);
	/* reception de la premiere ligne du processus precedent */
	printf(" [DEBUG] process -> %d \t reception de message du process %d \n", my_rank, my_rank - 1);
	MPI_Recv(premiere_ligne_du_buffer,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank-1,
		 0,
		 MPI_COMM_WORLD,
		 &status);
	printf(" [DEBUG] process -> %d \t reception de message du process %d ------ OK\n", my_rank, my_rank - 1);
      }else{
	/* envoie de la premiere ligne au process precedent */
	printf(" [DEBUG] process -> %d \t envoie de message à process %d \n", my_rank, my_rank - 1);
	MPI_Send(premiere_ligne_calculee,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank-1,
		 0,
		 MPI_COMM_WORLD);
	/* reception de la premiere ligne du processus precedent */
	printf(" [DEBUG] process -> %d \t reception de message du process %d \n", my_rank, my_rank - 1);
	MPI_Recv(premiere_ligne_du_buffer,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank-1,
		 0,
		 MPI_COMM_WORLD,
		 &status);
	/* envoie de la derniere ligne calculé au process suivant */
	printf(" [DEBUG] process -> %d \t envoie de message à process %d \n", my_rank, my_rank + 1);
	MPI_Send(derniere_ligne_calculee,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank+1,
		 0,
		 MPI_COMM_WORLD);
	/* reception de la derniere ligne */
	printf(" [DEBUG] process -> %d \t reception de message du process %d \n", my_rank, my_rank + 1);
	MPI_Recv(derniere_ligne_du_buffer,
		 w,
		 MPI_UNSIGNED_CHAR,
		 my_rank+1,
		 0,
		 MPI_COMM_WORLD,
		 &status);
      }
    }
    /* ==================================== 
     *            CONVOLUTION
     * ====================================
     */  
    printf(" [DEBUG] convolution ....\n");
    convolution( filtre, premiere_ligne_du_buffer, lh, w);
      
  }
  
  
  printf(" [DEBUG] rassemblement ....\n");
  /* ==================================== 
   *            RASSEMBLEMENT
   * ====================================
   */ 
  if(my_rank == 0){
    MPI_Gather(tmp,
	       l,
	       MPI_UNSIGNED_CHAR,
	       r.data,
	       l,
	       MPI_UNSIGNED_CHAR,
	       0,
	       MPI_COMM_WORLD);    
  }else{
    MPI_Gather(tmp+w,
	       l,
	       MPI_UNSIGNED_CHAR,
	       r.data,
	       l,
	       MPI_UNSIGNED_CHAR,
	       0,
	       MPI_COMM_WORLD);
  }


  printf(" [DEBUG] finalisation ....\n");
  /* ==================================== 
   *            FINALISATION
   * ====================================
   */ 
  if(my_rank == 0){
    /* Sauvegarde du fichier Raster */
    { 
      char nom_sortie[100] = "";
      sprintf(nom_sortie, "post-convolution_filtre%d_nbIter%d.ras", filtre, nbiter);
      sauve_rasterfile(nom_sortie, &r);
    }

    /* fin du chronometrage */
    fin = my_gettimeofday();
    printf("Temps total de calcul : %g seconde(s) \n", fin - debut);
  }
  MPI_Finalize();

  return 0;
}

