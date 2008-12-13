/* client.h */

/* les headers */
void print_reponse(chat_request * message);
void welcome (chat_request * message);
int est_quit_ack(chat_request * reponse_quit);
void cancel_threads_communication();
void kill_client(int code_erreur);
void deconnexion();
void * envoie_message();
void * reception_message();
void recevoir_reponse (chat_request * reponse);
void envoyer_message(chat_request * message);
void init_deroutement();
void init_socket_communication(char * hostname);
void set_requete(chat_request * requete, char * pseudo, char * message);
void join_server();
void init_threads_communication(chat_request *requete, chat_request * reponse);
