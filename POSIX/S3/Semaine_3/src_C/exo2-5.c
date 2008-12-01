#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define _POSIX_SOURCE 1
#define FILENAME "exo2-5.tmp"


/* TRAITEMENT DU SIGNAL SIGUSR1 */
void sig_hand(int sig){
  char c;
  FILE * fichier;
  if (sig == SIGUSR1){    
    fprintf(stderr,"Traitement de SIGUSR1...\n");
    /* j'ouvre le fichier cree par mon pere */
    if ( (fichier = fopen(FILENAME,"r")) == NULL){
      fprintf(stderr,"Erreur: fopen\n");
      exit (1);
    }
    
    /* j'affiche le contenu du fichier que mon */
    /* pere vient de remplir */
    fprintf(stderr,"Le contenu du fichier cree par le pere :\n");
    while ( (c=fgetc(fichier)) != EOF)
      printf("%c",c);
    
    /* fermer le fichier */
    fclose(fichier);
  }
}


int main (int argc, char * argv[]){
  
  sigset_t sig_set;
  pid_t fils;
  FILE * fichier;
  struct sigaction action;
    
  /* Creer un fichier */
  if ( (fichier = fopen(FILENAME,"w")) == NULL){
    fprintf(stderr,"Erreur: fopen\n");
    exit (1);
  }


  /* on doit masquer le signal SIGUSR1          */
  /* comme ca meme si le pere envoie le signal  */
  /* au fils et que le fils n est pas enore en  */
  /* mesure de le recevoir, il sera mis dans la */
  /* liste des signaux pendants                 */
  sigemptyset(&sig_set);
  sigaddset(&sig_set, SIGUSR1);
  sigprocmask(SIG_SETMASK, &sig_set, NULL);

  /* Creer un fils */
  if ((fils=fork()) == 0){
    /* je suis le fils */
    fprintf(stderr, "Ici le fils %d\n",getpid());
    /* vider l ensemble des signaux */
    sigfillset(&sig_set);
    
    /* ajouter le signal SIGUSER1 */
    sigdelset(&sig_set, SIGUSR1);
    
    /* changement du traitement du SIGUSR1 */
    action.sa_flags = 0;
    action.sa_handler = sig_hand;
    
    /* activer le traitement definie dans la segaction */
    /* action lorsque le signal SIGUSR1 est recu */
    sigaction(SIGUSR1, &action, NULL);
    
    
    /* j'attend que mon pere */
    /* m'envoie le signal SIGUSR1 */
    fprintf(stderr,"je suis le fils j attend que mon pere m'envoie un SIGUSR1\n");
    sigsuspend(&sig_set);

    /* Je me termine */
    exit(0);
  }
  
  fprintf(stderr,"Je suis le pere\n");
  /* Je suis le pere je met un message dans */
  /* le fichier que mon fils affichera */
  if (fputs("Ce que le pere a ecrit",fichier)==EOF){
    fprintf(stderr,"Erreur: fputs\n");
  }
  
  /* fermer le fichier */
  fclose(fichier);
  
  sleep(0);
  /* Je demande a mon fils de commencer son boulot */
  kill(fils, SIGUSR1);
  
  return 0;
  
}
