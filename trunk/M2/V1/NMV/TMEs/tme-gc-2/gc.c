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
static int nbO = 0;

static int nbThread = 0;

static struct object_header liste_obj_alloues;
static struct object_header liste_obj_atteints;

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
  struct object_header liste_obj_alloues; 

  // La liste des objets atteigniables
  struct object_header liste_obj_atteints;

  // La liste des objets racines
  struct object_header liste_racines;

  // La taille deja allouer
  int size_allocated;

  // Fini de rechercher les racines
  int ready_to_collect;

  int nbto;
  
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

int nbElt(struct object_header *header) {
  int cpt = 0;
  struct object_header *elt = header->next;

  while(elt != header) {
    cpt++;
    elt = elt -> next;
  } 

  return cpt;
}

void mark(struct object_header *header) {
  void *ptr;
  
  // Si objet n'est pas deja marquer
  if (header -> color != NOIR) {
    // Alors le marquer et l'ajouter au objets atteigniable
    header -> color = NOIR;


    // Suppresion de l'objet de la liste des objets alloues ou racines
    header -> prev -> next = header -> next;
    header -> next -> prev = header -> prev;
    header -> next = header -> prev = header;
    
    // Ajout de l'objet a la liste des atteints
    header -> next =  liste_obj_atteints.next;
    header -> prev = &liste_obj_atteints;
    liste_obj_atteints.next -> prev  = header;
    liste_obj_atteints.next          = header;
    
    
    // Puis parcours l'objet a la recherche des references
    for(ptr = toObject(header);ptr < toObject(header) + header -> object_size ; ptr++) {
      struct object_header *ref = toHeader(ptr);
      // Si une reference alors appeler mark dessus
      if (ref != NULL) { 
	mark(ref);
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

  nbO++;
  tls.nbto++;

  // Ajout du nouvelle entete dans la liste globale de tous les objets geres par la thread
  header -> next =  tls.liste_obj_alloues.next;
  header -> prev = &tls.liste_obj_alloues;
  tls.liste_obj_alloues.next -> prev  = header;
  tls.liste_obj_alloues.next          = header;

  // Met a jours la quantite de memoire alloue par le thread
  tls.size_allocated += size;

  // Si la taille alloue est > 4Mo alors demande une collection
  if (tls.size_allocated > (64 * 1024 * 1024)) {

    printf("gc tls.nbto = %d  |  nbElt = %d\n",tls.nbto,nbElt(&tls.liste_obj_alloues));
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
  char **cur = __builtin_frame_address(0);//down_stack();           // TODO : trouver le bas de la pile
  int nbR = 0; // debug
  int nb  = 0; // debug

  // Prend le mutex
  pthread_mutex_lock(&thread_mutex);
  if (req_collect == 0) {  // Si pas de demande de collection par le gcmalloc
    pthread_mutex_unlock(&thread_mutex); // 
    return;                // Alors libere le mutex et ne rien faire
  }

  printf("hc *1* tls.nbto = %d  |  nbElt = %d\n",tls.nbto,nbElt(&tls.liste_obj_alloues));
  
  tls.liste_racines.next = tls.liste_racines.prev = &tls.liste_racines;
  // Sinon parcours de la pile a la recherche des racines
  while ((char*) cur < tls.top_stack){ // HYPOTHESE : la pile croit vers des adresses hautes
    if ((racine = toHeader(*cur))){    // Si une racine 
      // Alors ajout dans la liste des racines si pas deja presente 
      if (racine -> is_racine == 0) {
	racine -> is_racine = 1;
	nbR++; // debug

	// Suppresion de la racine de la liste des objets alloues
	racine -> prev -> next = racine -> next;
	racine -> next -> prev = racine -> prev;
	racine -> next = racine -> prev = racine;

	// Ajout de la racine de a la liste des racines
	racine -> next =  tls.liste_racines.next;
	racine -> prev = &tls.liste_racines;
	tls.liste_racines.next -> prev  = racine;
	tls.liste_racines.next          = racine;

	printf("hc *#* tls.nbto = %d  |  nbElt = %d   | nbR = %d |  racine =%p\n",tls.nbto,nbElt(&tls.liste_obj_alloues),nbR,racine);
      }
    }
    nb++; // debug
    cur++;
  }
  
  printf("hc *2* tls.nbto = %d  |  nbElt = %d\n",tls.nbto,nbElt(&tls.liste_obj_alloues));
  printf("----------%p Nb = %d   |   NbR = %d | TailleRacine = %d  | ptrRacine = %p\n ",&tls,nb,nbR,nbElt(&tls.liste_racines),&tls.liste_racines);

  // Si dernier mutateur
  nb_ready++;
  if (nb_ready == nbThread) {
    // Alors reveille le collecteur
    printf("Demande de collection.\n");
    pthread_cond_signal(&cond_collect);
  }
 
  // Et dans tous les cas attendre la fin de la collection
  printf("Attend fin de collection.\n");
  pthread_cond_wait(&cond,&thread_mutex);
  printf("Fin attend de collection.\n");
  sleep(2);

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
    struct thread_descriptor *thread_courrant = all_threads.next;
    while (thread_courrant != &all_threads) {
      printf("++++++++ %p  | ptrRacine = %p \n",thread_courrant,&thread_courrant->liste_racines);

      // Marquer les objet atteigniable par le thread
      struct object_header *racine = thread_courrant->liste_racines.next;
      struct object_header *tmp    = thread_courrant->liste_racines.next;
      int nb = 0;
      printf("      Marquer les objets atteigniable.\n");
      printf("      NbRacine = %d.\n",nbElt(racine));
      while (racine != &(thread_courrant->liste_racines)) {
	printf("      %d Appel a mark\n",++nb);
	tmp = racine -> next;	
	mark(racine);
	racine = tmp;	
      }
      printf("      Apres Mark NbRacine = %d.\n",nbElt(&thread_courrant->liste_racines));
      thread_courrant = thread_courrant -> next;
    }

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

  tls.liste_obj_alloues.next  = tls.liste_obj_alloues.prev  = &tls.liste_obj_alloues;
  tls.liste_obj_atteints.next = tls.liste_obj_atteints.prev = &tls.liste_obj_atteints;
  tls.liste_racines.next      = tls.liste_racines.prev      = &tls.liste_racines;

  nbThread++;

  pthread_mutex_unlock(&thread_mutex);
}

// détache le thread, i.e. retire le tls de all_threads
void detach_thread() {
  pthread_mutex_lock(&thread_mutex);
  dprintf("thread", "Detach thread with tls at: %p", &tls);
  tls.next->prev = tls.prev;
  tls.prev->next = tls.next;
  tls.next = tls.prev = &tls;

  // vider racine, alloues, atteints
  /*
    !!!!!!!!!!
    !!!!!!!!!!
    !!!!!!!!!!
   */

  nbThread--;

  pthread_mutex_unlock(&thread_mutex);
}

// initialisation du gc: création du thread collecteur et initialisation de la variable all_threads
// vous devez completer cette fonction pour y ajouter vos structures de données
void initialise_gc() {
  pthread_t tid;
  all_threads.prev = all_threads.next = &all_threads;
  liste_obj_atteints.prev = liste_obj_atteints.next = &liste_obj_atteints;
  liste_obj_alloues.prev  = liste_obj_alloues.next  = &liste_obj_alloues;
  pthread_create(&tid, 0, &collector, 0);
}


