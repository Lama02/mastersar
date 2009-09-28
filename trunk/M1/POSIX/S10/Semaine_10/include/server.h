/* server.h*/

/* les headers*/
void stop_communication(int scom);
void stop_server();
int est_join(chat_request * req);
int est_quit(chat_request * req);
int get_free_index();
void del_connecte (int scom);
void put_message(int scom, chat_request * message);
void pop_message( chat_request * message);
void recevoir_requete(int scom, chat_request * requete);
void envoyer_reponse(int scom, chat_request *reponse);
void * diffuser_messages();
void deconnexion(int scom, char * pseudo);
void traitement_quit(int scom, char * pseudo);
void add_connecte(int scom, char * pseudo);
void traitement_join(int scom, char * pseudo);
void * traitement_req(void * scom);
void init_deroutement();
void init_file_messages();
void init_socket_connexion();
int est_server_down(chat_request * server_down);
int wait_client();
void start_thread_diffusion_messages();
void set_requete(chat_request * requete, char * pseudo, char * message);
void server_full(int scom);
int new_thread_for_client(int scom);

