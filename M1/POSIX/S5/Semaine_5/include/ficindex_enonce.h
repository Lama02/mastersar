/* Ficindex_enonce.h */
#ifndef __Ficindex
#define __Ficindex

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

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
int indopen(const char *path, int flags, int ind, mode_t mode);

/**
 * Ferme un fichier indexe.
 * 	fd  le descripteur du fichier a fermer
 * Renvoie -1 en cas de pb, 0 sinon.
 */
int indclose(int fd);


/**
 * Lit des donnees dans un fichier indexe.
 *	fd	descripteur du fichier lu
 *	buf     ptr vers les donnees lues
 *	nbytes  nb d'octets a lire 
 *		(nbytes doit etre >= a la taille d'index)
 * Renvoie le nb d'octets lus, 0 si fin de fichier, -1 en cas de pb.
 */
ssize_t indread(int fd, void *buf, size_t nbytes);

/**
 * Ecrit des donnees dans un fichier indexe.
 *	fd	descripteur du fichier modifie
 *	buf     ptr vers les donnees la ecrire
 *	nbytes  nb d'octets a ecrire 
 *		(nbytes doit etre >= a la taille d'index)
 * Renvoie le nb d'octets ecrits, -1 en cas de pb.
 */
ssize_t indwrite(int fd, const void *buf, size_t nbytes);

/**
 * Deplace le curseur dans un fichier indexe.
 *	fd	descripteur du fichier
 *	offset  deplacement a effectuer (en nb de structures index)
 *	whence  positionnement initial du curseur 
 * Renvoie le deplacement effectif du curseur (en nb d'octets), -1 en cas de pb.
 */
off_t indlseek(int fd, off_t offset, int whence);

/**
 * Recherche un enregistrement dans un fichier indexe.
 *	fd	descripteur du fichier
 *	cmp	fonction de comparaison
 *	key	cle de recherche
 * Renvoie le 1er elt trouve pr lequel la comparaison est correcte, NULL sinon.
 */
void *indsearch(int fd, int (*cmp)(void *, void *), void *key);

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
int indxchg(char *oldfic, char *newfic, unsigned int newind);

#endif
