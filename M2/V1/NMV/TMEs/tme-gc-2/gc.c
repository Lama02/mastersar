#include "gc.h"
#include "alloc.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "debug.h"

#define NOIR  1
#define BLANC 0


// Si positionne a  
//   1 tous les handshake de tous les threads doivent etre lances
//   0 normal
// ATTENTION : A POROTEGER PAR UN VERROU
static int req_collect = 0;
static int nb_ready    = 0;

// Condition 
static pthread_cond_t   cond              = PTHREAD_COND_INITIALIZER;
static pthread_cond_t   cond_collect      = PTHREAD_COND_INITIALIZER;


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


  // La liste des objets du thread
  struct object_header *liste_objets; 

  // La liste des objets racines
  struct object_header *liste_racines;

  // La liste des objets racines
  struct object_header *liste_atteignables;

  // La taille deja allouer
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
// une simplification pour les liste doublement chaînées circulaires
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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*             TODO                 */
char** down_stack() {
  return __builtin_frame_address(0);
}
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void mark(struct object_header *header) {
  void *ptr;
  if (header -> color != NOIR) {
    for(ptr = toObject(header);ptr < ptr + header -> object_size ; ptr++) {
      struct object_header *ref = toHeader(ptr);
      if (ref != 0) {
	ref -> color = NOIR;
	ref -> next  =   tls.liste_atteignables;
	ref -> prev  =   NULL;
	if (tls.liste_atteignables != NULL)
	  tls.liste_atteignables -> prev = ref;
	tls.liste_atteignables = ref; 
	mark(toHeader(ref));
      }
    }
  }
}

// la fonction gcmalloc, vous devez remplir cette fonction
void *gcmalloc(unsigned int size) {
  // l'allocation se fait en suivant l'algo de hash de Boehm
  struct object_header *header = pre_malloc(size);

  // ensuite, vous devrez mettre cette entête dans une liste des objets vivants...
  // vous pouvez prendre un verrou ici, mais vous pouvez aussi utiliser le tls: l'ensemble des objets vivants
  // est stocké dans l'ensemble des tls. Ca vous évite un verrou de plus

  // Prend le mutex
  pthread_mutex_lock(&thread_mutex);

  // Ajout du nouvelle entete dans la liste globale de tous les objets geres par la thread
  header -> next = tls.liste_objets;
  header -> prev = NULL;
  if (tls.liste_objets != NULL)
    tls.liste_objets -> prev = header;
  tls.liste_objets = header;

  // Met a jours la quantite de memoire alloue par le thread
  tls.size_allocated += size;

  // Si la taille alloue est > 4Mo alors demande une collection
  if (tls.size_allocated > (64 * 1024 * 1024)) {
    req_collect = 1;
  }

  // Libere le mutex
  pthread_mutex_unlock(&thread_mutex);

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
  struct object_header * racine;       // Adresse de la racine 
  char **cur = down_stack();           // TODO : trouver le bas de la pile

  // Prend le mutex
  pthread_mutex_lock(&thread_mutex);
  if (req_collect == 0) {  // Si pas de demande de collection par le gcmalloc
    pthread_mutex_unlock(&thread_mutex); // 
    return;                // Alors libere le mutex et ne rien faire
  }
  
  // Sinon parcours de la pile a la recherche des racines
  while ( (char*)cur > tls.top_stack){ // HYPOTHESE : la pile croit vers des adresses basses
    if ((racine = toHeader(*cur))){    // Si une racine 
      // Alors ajout dans la liste des racines 
      // is_racine;
      racine -> next = tls.liste_racines;
      racine -> prev = NULL;
      if (tls.liste_racines != NULL)
	tls.liste_racines -> prev = racine;
      tls.liste_racines = racine;
    }
    cur--;
  }


  nb_ready++;
  // Si dernier mutateur
  if (nb_ready == 7) {
    // Alors reveille le collecteur
    printf("Demande de collection.\n");
    pthread_cond_signal(&cond_collect);
  }
 
  // Et dans tous les cas attendre la fin de la collection
  printf("Attend fin de collection.\n");
  pthread_cond_wait(&cond,&thread_mutex);
  printf("Fin attend de collection.\n");

  // Enfin reinisialise le mutateur
  tls.size_allocated = 0;

  // Libere le mutateur
  pthread_mutex_unlock(&thread_mutex);
}

// le thread collecteur. Quand une collection est requise, il doit se réveiller et parcourir le graphe
// des objets atteignables puis supprimer tous ceux qui n'ont pas été atteints
static void *collector(void *arg) {
  printf("**************************** COLLECTOR ***************************\n");
  // Boucle infinie
  while (1) {
    // Prend le mutex
    pthread_mutex_lock(&thread_mutex);

    // Si pas de demande de collection
    while (req_collect == 0) {
      // Alors dort en attendant d'etre reveille par un mutateur
      printf("   Attend requete de collection.\n");
      pthread_cond_wait(&cond_collect,&thread_mutex);
    }

    // Sinon debut de collection
    printf("   Debut collection.\n");
    // Pour chaque thread mutateur
    struct thread_descriptor *thread_courrant = &all_threads;
    do {
      // Marquer les objet atteigniable par le thread
      struct object_header *racine;
      for (racine  = thread_courrant -> liste_racines;
	   racine != NULL;
	   racine  = racine -> next) {
	mark(racine);
      }
      // Puis parcourir la liste des objets alloue par le thread
      struct object_header *obj;
      for (obj  = thread_courrant -> liste_objets;
	   obj != NULL;
	   obj  = obj -> next) {
	// Et libere l'objet si pas atteigniable
	if (obj -> color == NOIR) {
	  obj -> color = BLANC;
	}
	else {
	  // Erreur de segmentation
/* 	  if (obj -> prev != NULL) */
/* 	    obj -> prev -> next = obj -> next; */
/* 	  if (obj -> next != NULL) */
/* 	    obj -> next -> prev = obj -> prev; */
/* 	  pre_free(toObject(obj)); */
	}
      }
      thread_courrant = thread_courrant -> next;
    } while (thread_courrant -> next != &all_threads);

    printf("   Fin collection.\n");

    // Reinisialisation
    req_collect = 0;
    nb_ready    = 0;

    // Reveille les mutateurs
    printf("   Reveille mutateur.\n");
    pthread_cond_broadcast(&cond);

    // Libere le mutex
    pthread_mutex_unlock(&thread_mutex);
  }
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


