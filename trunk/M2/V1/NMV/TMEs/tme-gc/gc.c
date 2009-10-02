#include "gc.h"
#include "alloc.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "debug.h"



/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*             TODO                 */
char** down_stack() {
  return NULL;
}
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


// Si positionne a  
//   1 tous les handshake de tous les threads doivent etre lances
//   0 normal
// ATTENTION : A POROTEGER PAR UN VERROU
int req_collect;

// Condition 
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


// descriptor de thread
// on a un descriptor par thread
// ils sont chaînés pour que le collecteur puisse se promener dedans
struct thread_descriptor {
  struct thread_descriptor *next;        // descripteur précédent
  struct thread_descriptor *prev;        // descripteur suivant
  char *                    top_stack;   // haut de pile
  // ajoutez vos champs de threads ici
  // vous avez besoin d'une structure pour mettre vos racines sur votre pile
  // et vous avez besoin de drapeau pour indiquer au collecteur que vous avez fini de
  // placer les racines la dedans
        
  // la liste globale des objets du thread
  struct object_header *liste_objets; 
  
  // la liste des objets racines
  struct object_header *liste_racines;

  // la taille deja allouer
  // par defaut initialise a 0. TODO c possible ca ?
  int size_allocated;

  // Fini de rechercher les racines
  int ready_to_collect;
  
};

// chaque thread a sa propre image de cette variable: ce sont des variables locales au thread
// d'un point technique, un segment est utilisé pour stocker les variables "thread local storage"
// pour gcc, accéder à la variable revient à passer par gs et prendre son adresse revient à trouver son adresse
// relativement au segment ds: cette adresse est donc la même pour tout les threads
static __thread struct thread_descriptor tls;
// amorce de la liste chaînée de tous les threads. Ce noeud est vide, il sert de référence. C'est juste
// une simplification pour les listes doublement chaînées circulaires
static struct thread_descriptor          all_threads;
// et un petit mutex lorsqu'on touche à cette liste
static pthread_mutex_t                   thread_mutex = PTHREAD_MUTEX_INITIALIZER; 

#ifdef _DEBUG
// permet d'afficher les threads actifs
// ça vous donne un exemple de parcours de la liste chaînée all_threads
void print_threads() {
  pthread_mutex_lock(&thread_mutex);
  dprintf("thread", "Current threads:");
  for(struct thread_descriptor *cur=all_threads.next; cur!=&all_threads; cur=cur->next)
    dprintf("thread", "\tthread with tls at: %p", &cur);
  pthread_mutex_unlock(&thread_mutex);
}
#else
#define print_threads()
#endif



// la fonction gcmalloc, vous devez remplir cette fonction
void *gcmalloc(unsigned int size) {
  // l'allocation se fait en suivant l'algo de hash de Boehm
  struct object_header *header = pre_malloc(size);

  // ensuite, vous devrez mettre cette entête dans une liste des objets vivants...
  // vous pouvez prendre un verrou ici, mais vous pouvez aussi utiliser le tls: l'ensemble des objets vivants
  // est stocké dans l'ensemble des tls. Ca vous évite un verrou de plus


  // ajout du nouvelle entete dans la liste globale de tous les objets geres par la thread
  pthread_mutex_lock (&thread_mutex);
  header -> next =   tls.liste_objets;
  header -> prev =   tls.liste_objets -> prev;
  tls.liste_objets -> prev = header;
  tls.liste_objets = header;
  // au bout de PAGE_SIZE (4096) Mo on lance la collection   
  tls.size_allocated += size;
  pthread_mutex_unlock(&thread_mutex);
  if (tls.size_allocated >= 4096) { 
    pthread_mutex_lock(&thread_mutex);
    req_collect = 1;
    pthread_mutex_unlock(&thread_mutex); 
  }
  
  // pour le retour, l'utilisateur est intéressé par l'objet, pas par son entête
  return toObject(header);
}

// la fonction writeBarrier qui doit assurer l'invariant tri-couleurs de boehm
void _writeBarrier(void *dst, void *src) {
  // à faire
  struct object_header *header_dst = toHeader(dst);  // entête de la destination
  struct object_header *header_src = toHeader(src);  // entête de la source
  
  assert(header_dst && header_src);                  // si vous n'avez pas de bug, cet invariant est respecté
}




// le handshake pour accumuler les racines du thread. Vous les stockerez dans la variable tls
// celle-ci est ensuite accédée par le collecteur via la variable all_threads
void handShake() {
  struct object_header * racine;       // adresse de la racine 
  char **cur = down_stack();          // TODO : trouver le bas de la pile
  
  
  // attendre que req_collect soit positionne a 1
  pthread_mutex_lock (&thread_mutex);
  while (req_collect != 1) {
    pthread_cond_wait(&cond,&thread_mutex);
  }
  // parcours de la pile a la recherche des racines
  while ( (char*)cur > tls.top_stack){ // HYPOTHESE : la pile croit vers des adresses basses
    if ((racine = toHeader(*cur))){        // si une racine 
      // alors ajout dans la liste des racines 
      racine -> next = tls.liste_racines;
      racine -> prev = tls.liste_racines -> prev;
      tls.liste_racines -> prev = racine;
      tls.liste_racines = racine;
    }
  }
  tls.ready_to_collect = 1;
  pthread_mutex_unlock(&thread_mutex);


  // attendre la fin de la collection
  pthread_mutex_lock (&thread_mutex);
  while (req_collect != 0) {
    pthread_cond_wait(&cond,&thread_mutex);
  }
  pthread_mutex_unlock(&thread_mutex);

}



// le thread collecteur. Quand une collection est requise, il doit se réveiller et parcourir le graphe
// des objets atteignables puis supprimer tous ceux qui n'ont pas été atteints
static void *collector(void *arg) {
  
  
  
  // reveille tous les thread mutateur
  pthread_mutex_lock(&thread_mutex);
  req_collect = 0;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&thread_mutex); 
  
  return 0;
}


struct stack_frame {
  struct stack_frame *next;
  char               *co;
};

// attache un nouveau thread au gc. Techniquement, ajoute son tls à all_threads
void attach_thread(void *top) {
  // première étape: trouve le haut de la pile
  //	struct stack_frame *top = __builtin_frame_address(0);
  //	while(top->next)
  //		top = top->next;

  pthread_mutex_lock(&thread_mutex);
  tls.top_stack = (char *)top;
  dprintf("thread", "Attach thread with tls at %p and top stack at %p", &tls, tls.top_stack);
  (tls.prev = all_threads.prev)->next = &tls;
  (tls.next = &all_threads)->prev = &tls;
  pthread_mutex_unlock(&thread_mutex);
}

// détache le thread, i.e. retire le tls de all_threads
void detach_thread() {
  pthread_mutex_lock(&thread_mutex);
  dprintf("thread", "Detach thread with tls at: %p", &tls);
  tls.next->prev = tls.prev;
  tls.prev->next = tls.next;
  tls.next = tls.prev = &tls;
  pthread_mutex_unlock(&thread_mutex);
}

// initialisation du gc: création du thread collecteur et initialisation de la variable all_threads
// vous devez completer cette fonction pour y ajouter vos structures de données
void initialise_gc() {
  pthread_t tid;
  all_threads.prev = all_threads.next = &all_threads;
  pthread_create(&tid, 0, &collector, 0);
}


