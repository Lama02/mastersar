#define _POSIX_SOURCE 1

#include "ficindex_enonce.h"

#define TAILLE_BUF 1000
/* nombre maximum de fichier qu un processus peut ouvrir */
#define OPEN_MAX 100

int sizeindex[OPEN_MAX];

/**
 * Ouvre un fichier indexe.
 * Fichier cree ou ecrase => ecrire la taille de l'index en debut de fichier
 * Sinon => lire la taille de l'index en debut de fichier
 * 	path   nom du fichier a ouvrir
 * 	flags  mode d'ouverture du fichier
 * 	ind    taille de la structure index (ne sert que si le fichier est cree ou ecrase)
 * 	mode   droits d'acces au fichier
 * Renvoie le descripteur du fichier ouvert, -1 en cas de pb.
 */
int indopen(const char *path, int flags, int ind, mode_t mode){
  
  int tocreate=0;
  struct stat st;
  int fd;
  
  if (flags & O_TRUNC) tocreate = 1;

  if (flags & O_CREAT){
    if(stat(path,&st) < 0)
      tocreate = 1;
  }
  
  if ((fd = open(path, flags, mode)) < 0) return -1;
  if(tocreate){
    write(fd, &ind, sizeof(int));
  }
  else
    read(fd, &ind, sizeof(int));
   
  sizeindex[fd] = ind;
  return fd;
}



/**
 * Ferme un fichier indexe.
 * 	fd  le descripteur du fichier a fermer
 * Renvoie -1 en cas de pb, 0 sinon.
 */
int indclose(int fd){

  return close(fd);
}



/**
 * Lit des donnees dans un fichier indexe.
 *	fd	descripteur du fichier lu
 *	buf     ptr vers les donnees lues
 *	nbytes  nb d'octets a lire 
 *		(nbytes doit etre >= a la taille d'index)
 * Renvoie le nb d'octets lus, 0 si fin de fichier, -1 en cas de pb.
 */
ssize_t indread(int fd, void *buf, size_t nbytes){

/* si la taille du buffer est petite */
  if (nbytes < sizeindex[fd]){
      errno = EBADF;
      return -1;
  }
  /* si tout va bien on se contente de lire */
  return (read(fd, buf, sizeindex[fd]));
}


/**
 * Ecrit des donnees dans un fichier indexe.
 *	fd	descripteur du fichier modifie
 *	buf     ptr vers les donnees la ecrire
 *	nbytes  nb d'octets a ecrire 
 *		(nbytes doit etre >= a la taille d'index)
 * Renvoie le nb d'octets ecrits, -1 en cas de pb.
 */
ssize_t indwrite(int fd, const void *buf, size_t nbytes){

  /* si la taille du buffer est petite */
  if (nbytes < sizeindex[fd]){
    errno = EBADF;
    return -1;
  }
  /* si tout va bien on se contente de lire */
  return (write(fd, buf, sizeindex[fd]));
}



/**
 * Deplace le curseur dans un fichier indexe.
 *	fd	descripteur du fichier
 *	offset  deplacement a effectuer (en nb de structures index)
 *	whence  positionnement initial du curseur 
 * Renvoie le deplacement effectif du curseur (en nb d'octets), -1 en cas de pb.
 */
off_t indlseek(int fd, off_t offset, int whence){
  
  /* le debut de notre fichier est situe juste 
   * apres le premier champ contenant la taille de 
   * la structure index
   */
  if(whence==SEEK_SET){
    whence += sizeof(int); 
  }
  return lseek(fd, offset * sizeindex[fd], whence);
}



/**
 * Recherche un enregistrement dans un fichier indexe.
 *	fd	descripteur du fichier
 *	cmp	fonction de comparaison
 *	key	cle de recherche
 * Renvoie le 1er elt trouve pr lequel la comparaison est correcte, NULL sinon.
 */
void *indsearch(int fd, int (*cmp)(void *, void *), void *key){
  int i, nb_struct;
  struct stat st;
  void * buf = malloc(sizeindex[fd]);
  
  /* nombre de structures */
  if (fstat(fd, &st) == -1) return NULL;
  nb_struct = (st.st_size - sizeof(int)) / sizeindex[fd];

  /* nous allons parcourir l'ensemble des enregistrements */
  for (i=0; i< nb_struct; i++){
    indread(fd, buf, sizeindex[fd]);
    if ( cmp(buf, key) == 1){
      /* la structure match */
      return buf;
    }
  }
  return NULL;
}



/**
 * Change l'indexation d'un fichier.
 * Les donnees sont recopiees dans un nouveau fichier avec les memes droits d'acces.
 * 	oldfic	fichier a reindexer
 *	newfic	fichier resultat
 *	newind	nouvelle taille de structure index
 * Si la nouvelle taille est < a la precedente, les donnees supplementaires sont perdues.
 * 	ex : 	'ftoto' contient des struct toto { int pi; char ps[2]; };
 *		'ftata' contient des struct tata { int pj; };
 *		indxchg("ftoto", "ftata", sizeof(struct tata));
 *		=> les donnees correspondant a ps seront perdues
 * Renvoie -1 en cas de pb, 0 sinon.
 */
int indxchg(char *oldfic, char *newfic, unsigned int newind){
  int n;
  int fd_oldfic, fd_newfic;
  int * buf_oldfic = malloc(sizeof(int));
  struct stat st; /* pour recuperer les droits d'acces */

  /* comparaison des tailles d'index */
  if ( (fd_oldfic = open(oldfic, O_RDONLY)) == -1){
    return -1;
  }
  if (read(fd_oldfic, buf_oldfic, sizeof(int)) == -1){
    return -1;
  }
  
  /* comparer la taille d'index du fichier oldfic a la nouvelle taille newind */
  if ( newind == *buf_oldfic ){
    /* recuperer les droits d'acces du fichier oldfic */
    if (fstat(fd_oldfic, &st) == -1) return -1;
    /* on recopie le fichier oldfic dans newfic */
    if ( (fd_newfic = indopen(newfic, O_WRONLY | O_CREAT| O_EXCL, newind, st.st_uid |st.st_gid )) == -1){
      return -1;
    }
    /* tant que le fichier oldfic contient des donnees */
    /* on les recopie */
    free(buf_oldfic);
    buf_oldfic = malloc(TAILLE_BUF * sizeof(int));
    /* on lit le contenu du fichier oldfic a partir de la 1ere struct */
    while ((n = read(fd_oldfic, buf_oldfic, TAILLE_BUF)) != -1){
      /* on ecrit dans le fichier newfic */
      if (write (fd_newfic, buf_oldfic, TAILLE_BUF) == -1 ){
	return -1;
      }
    }
    if (n == -1){
      /* on a rencontre un probleme pendant la lecture */
      return -1;
    }
  }
  
  if (1){
    /* si la taille d'index du fichier oldfic est plus grande que newind */
    /* TODO */
    return 0;
  }
  if (1){
    /* si la taille d'index du fichier oldfic est plus petite que newind */
    /* TODO */
    return 0;
  }
}

