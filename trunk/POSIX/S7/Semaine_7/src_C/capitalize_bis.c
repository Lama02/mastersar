#define _POSIX_SOURCE 1

#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#define TAILLE sizeof(char)


int main(int argc, char* argv[]){
  char char_lu;
  /* pour stocker les infos sur la file */
  struct msqid_ds *buf = (struct msqid_ds *) malloc(sizeof(struct msqid_ds));
  
  struct message {
    long type;
    char mon_char;
  }msg_snt, msg_rcv;
  
  int msgid ;
  
  /* Creation d'une cle */
  key_t cle ;
  char path[14] = "file_msg";
  char code = 'Q';
  cle = ftok(path,code);

  
  /* Creer une nouvelle file de messages */
  if ( (msgid = msgget(cle, 0666 | IPC_CREAT)) == -1){
    perror("msgget");
    return 1;
  }
  
  /* mettre les info (du systeme) de la file */
  /* dans la variable buf */
  msgctl(msgid,IPC_STAT,buf);
  
  
  if (fork()==0){
    /* le fils */
    
    while(1){      
      /* destination du message recupere */
      if (msgrcv(msgid, &msg_rcv, sizeof(struct message), 0, 0) == -1){
	perror("msgrcv");
	
	/* detruire la file */
	msgctl(msgid,IPC_RMID,buf);
	return 1;
      }
      /* recuperer un message de la file */
      printf("%c", toupper(msg_rcv.mon_char)); 
    }
  }else{
    /* le pere */
    while(1){
      /* lire le message saisi */
      scanf("%c", &char_lu);
      /* creer le message a envoye */
      msg_snt.mon_char = char_lu;
      /* envoyer le char lu dans un message */
      if (msgsnd(msgid, &msg_snt, sizeof(struct message), 0) == -1){
	perror("msgsnd");
	
	/* detruire la file */
	msgctl(msgid,IPC_RMID,buf);
	return 1;
      }
    }
  }
  
  /* detruire la file */
  msgctl(msgid,IPC_RMID,buf);
  
  return EXIT_SUCCESS;
}
