#include "direct_computation.h"
#include "var_mpi.h"






/*********************************************************************************************
**********************************************************************************************
**********************************************************************************************

   Without Matrices

   **********************************************************************************************
   **********************************************************************************************
   *********************************************************************************************/

/* All the following functions use the mutual interaction principle (i.e. reciprocity). */




/*** For debugging only: we check if the positions are too close for direct computation: ***/
/* #define _CHECK_IF_POSITION_ARE_TOO_CLOSE_ */
#ifdef _CHECK_IF_POSITION_ARE_TOO_CLOSE_
#define CHECK_IF_POSITION_ARE_TOO_CLOSE(pos_x_tgt, pos_y_tgt, pos_z_tgt, pos_x_src, pos_y_src, pos_z_src) { \
    if (position_Are_too_close(pos_x_tgt, pos_y_tgt, pos_z_tgt, pos_x_src, pos_y_src, pos_z_src)){ \
      fprintf(f_output, "In file direct_computation.c: the two position are too close:\n"); \
      pos_xyz_Display(pos_x_tgt, pos_y_tgt, pos_z_tgt, f_output, low);	\
      fprintf(f_output, "\t and \t");					\
      pos_xyz_Display(pos_x_src, pos_y_src, pos_z_src, f_output, low);	\
      fprintf(f_output, "\n");						\
      FMB_ERROR_BRIEF();						\
    }									\
  }
#else 
#define CHECK_IF_POSITION_ARE_TOO_CLOSE(pos_x_tgt, pos_y_tgt, pos_z_tgt, pos_x_src, pos_y_src, pos_z_src) 
#endif 




/* 
 * Reception en mode non bloquant du tableau p_b_tmp de 
 * taille nb_bodies du  site pred 
 */
void mpi_irecv(bodies_t * p_b_tmp, long nb_bodies, int pred, MPI_Request * reqr){
  MPI_Irecv(p_b_tmp->p_pos_x,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  MPI_Irecv(p_b_tmp->p_pos_y,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  MPI_Irecv(p_b_tmp->p_pos_z,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);

  MPI_Irecv(p_b_tmp->p_fx,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  MPI_Irecv(p_b_tmp->p_fy,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  MPI_Irecv(p_b_tmp->p_fz,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  

  MPI_Irecv(p_b_tmp->p_values,
	    nb_bodies,
	    MPI_FLOAT,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
  
  MPI_Irecv(p_b_tmp->p_speed_vectors,
	    nb_bodies*sizeof(position_t),
	    MPI_UNSIGNED_CHAR,
	    pred,
	    0,
	    MPI_COMM_WORLD,
	    reqr);
#ifdef _DEBUG_
  printf("\t [DEBUG] +++++++++++ Proc %d \t Dans la fonction mpi_irecv \t p_b_tmp->p_pos_x[1] : %f \t p_b_tmp->nb_bodies : %ld \t nb_bodies : %ld\n",rank, p_b_tmp->p_pos_x[1], p_b_tmp->nb_bodies, nb_bodies);
#endif
  /*------ Fin de reception non bloquante ------*/
}



void mpi_send(bodies_t * p_b2, long nb_bodies, int succ){
  MPI_Send(p_b2->p_pos_x,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);
  MPI_Send(p_b2->p_pos_y,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);
  MPI_Send(p_b2->p_pos_z,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);

  MPI_Send(p_b2->p_fx,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);
  MPI_Send(p_b2->p_fy,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);
  MPI_Send(p_b2->p_fz,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);

  MPI_Send(p_b2->p_values,
	   nb_bodies,
	   MPI_FLOAT,
	   succ,
	   0,
	   MPI_COMM_WORLD);
  
  MPI_Send(p_b2->p_speed_vectors,
	   nb_bodies*sizeof(position_t),
	   MPI_UNSIGNED_CHAR,
	   succ,
	   0,
	   MPI_COMM_WORLD);
#ifdef _DEBUG_
  printf("\t [DEBUG] +++++++++++ Proc %d \t Dans la fonction mpi_send \t p_b2->p_pos_x[1] : %f \t p_b2->nb_bodies : %ld \t nb_bodies : %ld\n",rank, p_b2->p_pos_x[1], p_b2->nb_bodies, nb_bodies);
#endif
  
  /*------ Debut de reception non bloquante ------*/
}




/*********************************************************************************************
**********************************************************************************************

   bodies_Compute_own_interaction

   **********************************************************************************************
   *********************************************************************************************/


#ifdef _BODIES_SPLIT_DATA_


void 
bodies_Compute_own_interaction(bodies_t *FMB_RESTRICT p_b){

  MPI_Status status1;
  bodies_ind_t i,j;
  bodies_ind_t n = bodies_Nb_bodies(p_b);
  
  int etape = 0;
  
  int pred = (rank - 1 + p) % p; /* mon predecessur */
  int succ = (rank + 1) % p; /* mon successeur  */
  MPI_Request reqr; /* attente de la reception non bloquante */
  
  long nb_bodies = p_b->nb_bodies;
  
  
  /* 
   * Ce tableau contiendra les résultats de mon pred 
   * initialement, ce tableau contiendra les mêmes 
   * valeurs que p_b.
   */
  bodies_t b2; 
  bodies_t *FMB_RESTRICT p_b2 = &b2;
  
  /* 
   * Comme la reception se fait pendant les calculs, 
   * ce tableau sera utilisé pour eviter l'ecrasement 
   * du tableau b2 qui quant à lui est utilsé dans 
   * les calculs
   */
  bodies_t b_tmp; 
  bodies_t *FMB_RESTRICT p_b_tmp = &b_tmp;
  
  bodies_t *FMB_RESTRICT p_b2_old = NULL;
  
  /* 
   * Var utiles pour le second tableau b2.
   */
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_px2;
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_py2;
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_pz2;

  FMB_CONST VALUES_T *FMB_RESTRICT p_val2;

  COORDINATES_T *FMB_RESTRICT p_fx2;
  COORDINATES_T *FMB_RESTRICT p_fy2;
  COORDINATES_T *FMB_RESTRICT p_fz2;
  

  /* 
   * Var utiles pour le premier tableau b.
   */
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_px;
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_py;
  FMB_CONST COORDINATES_T *FMB_RESTRICT p_pz;

  FMB_CONST VALUES_T *FMB_RESTRICT p_val;
  COORDINATES_T pix, piy, piz, pjx, pjy, pjz;
  VALUES_T val_i, val_j;

  COORDINATES_T *FMB_RESTRICT p_fx;
  COORDINATES_T *FMB_RESTRICT p_fy;
  COORDINATES_T *FMB_RESTRICT p_fz;
  COORDINATES_T fix, fiy, fiz;

  REAL_T eps_soft_square = FMB_Info.eps_soft_square;
  
  /* on initialise les deux tableaux b2 et b_tmp 
   * les trois tableaux ont la même taille.
   */
  bodies_Initialize(&b2, p_b->nb_bodies);
  bodies_Initialize(&b_tmp, p_b->nb_bodies);
  b2.nb_bodies = p_b->nb_bodies;
  b_tmp.nb_bodies = p_b->nb_bodies;
  
  
  /* copie du tableau p_b dans p_b2 */
  bodies_Affect(p_b2, p_b);
  
#ifdef _DEBUG_
  printf("\t [DEBUG] +++++++++++ Proc %d \t p_b->p_pos_x[1] : %f \t p_b->nb_bodies : %ld \t nb_bodies : %ld \t p : %d\n",rank, p_b->p_pos_x[1], p_b->nb_bodies, nb_bodies, p);
  printf("\t [DEBUG] +++++++++++ Proc %d \t p_b2->p_pos_x[1] : %f \t p_b2->nb_bodies : %ld \t nb_bodies : %ld\n", rank, p_b2->p_pos_x[1], p_b2->nb_bodies, nb_bodies);
  printf("\t [DEBUG] +++++++++++ Proc %d \t Je suis dans la fonction bodies_Compute_own_interaction \n", rank);
#endif
  
  /* on boucle tant qu'il y a des données à recevoir 
   * avec p sites. On doit attendre p-1 receptions
   */
  while (etape < p){

#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ ########### Proc %d \t Debut de l'iteration %d \n", rank, etape);
#endif
    
    /* on commence a recevoir les messages.*/
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Avant le Irecv \n", rank);
#endif
    /* je reçoie en mode non bloquant 
     * les resultats de mon pred dans 
     * le tableau b_tmp
     */
    mpi_irecv(p_b_tmp, nb_bodies, pred, &reqr);
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Apres le Irecv \n", rank);
#endif
    
    /* on commence a envoyer les messages.*/
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Avant le Send \n", rank);
#endif
    /* j'envoie le contenu de mon tableau b2 
     * à mon successeur ((rank +1) mod p) 
     */
    mpi_send(p_b2, nb_bodies, succ);
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Apres le Send \n", rank);
#endif

#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Debut des calculs \n", rank);
#endif
    /*------ Partie calculs ------*/
    p_px = p_b->p_pos_x;
    p_px2 = p_b2->p_pos_x;
  
    p_py = p_b->p_pos_y;
    p_py2 = p_b2->p_pos_y;

    p_pz = p_b->p_pos_z;
    p_pz2 = p_b2->p_pos_z;
  
    p_val = bodies_Get_p_value(p_b, 0);
    p_val2 = bodies_Get_p_value(p_b2, 0);

    p_fx = p_b->p_fx;
    p_fx2 = p_b2->p_fx;

    p_fy = p_b->p_fy;
    p_fy2 = p_b2->p_fy;
  
    p_fz = p_b->p_fz;
    p_fz2 = p_b2->p_fz;
    
    if (etape == 0){
#ifdef _DEBUG_
      printf("\t [DEBUG] +++++++++++ Proc %d \t Debut de la %deme iteration\n", rank, etape);
#endif
      /*------ Debut de la premiere iteration de calcul ------*/
      /*
       * Cette boucle n'est correcte que pour la première itération.
       * En effet, le j par exemple dois commencer de 0 au lieu de i+1
       */
      for(i=0; i<n-1; i++){
	pix = p_px[i];
	piy = p_py[i];
	piz = p_pz[i];
	val_i = p_val[i];
      
	fix = p_fx[i];
	fiy = p_fy[i];
	fiz = p_fz[i];
	
    
	for (j=i+1;	 j<n;	 j++){
	  pjx = p_px2[j];
	  pjy = p_py2[j];
	  pjz = p_pz2[j];
	  val_j = p_val2[j];
      
	  CHECK_IF_POSITION_ARE_TOO_CLOSE(pix, piy, piz, pjx, pjy, pjz);       
	  DIRECT_COMPUTATION_MUTUAL_SOFT(pix, piy, piz,
					 pjx, pjy, pjz,
					 val_i,
					 val_j,
					 fix, fiy, fiz,
					 p_fx2[j], p_fy2[j], p_fz2[j],
					 pot_i, /* pas utilise */
					 p_pot[j], /* pas utilise */
					 eps_soft_square);
	}//    for (j=i+1;	 j<n;	 j++){
	
	/* la macro s'occupe de calculer les nouvelles valeurs de fix, fiy, fiz 
	 * puis stocke les nouveaux résultats dans les deux tableaux
	 */    
	p_fx[i] = fix; 
	p_fy[i] = fiy;
	p_fz[i] = fiz;   
      }//    for(i=0; i<n-1; i++){   
      /*------ Fin de la premiere iteration de calcul ------*/  

#ifdef _DEBUG_
      printf("\t [DEBUG] +++++++++++ Proc %d \t Fin de la %deme iteration \n", rank, etape);
#endif

    }else{// if (etape == 0)
#ifdef _DEBUG_
      printf("\t [DEBUG] +++++++++++ Proc %d \t Debut de l'iteration %d \n", rank, etape);
#endif
      for(i=0; i<n; i++){
	pix = p_px[i];
	piy = p_py[i];
	piz = p_pz[i];
	val_i = p_val[i];
	  
	fix = p_fx[i];
	fiy = p_fy[i];
	fiz = p_fz[i];
	
	for (j=0;	 j<n;	 j++){
	  pjx = p_px2[j];
	  pjy = p_py2[j];
	  pjz = p_pz2[j];
	  val_j = p_val2[j];
      
	  CHECK_IF_POSITION_ARE_TOO_CLOSE(pix, piy, piz, pjx, pjy, pjz);       
	  DIRECT_COMPUTATION_MUTUAL_SOFT(pix, piy, piz,
					 pjx, pjy, pjz,
					 val_i,
					 val_j,
					 fix, fiy, fiz,
					 p_fx2[j], p_fy2[j], p_fz2[j],
					 pot_i, /* pas utilise */
					 p_pot[j], /* pas utilise */
					 eps_soft_square);
	}//    for (j=0;	 j<n;	 j++){
	
	/* la macro s'occupe de calculer les nouvelles valeurs de fix, fiy, fiz */    
	p_fx[i] = fix; 
	p_fy[i] = fiy;
	p_fz[i] = fiz;   
      }// for(i=0; i<n; i++){   
#ifdef _DEBUG_
      printf("\t [DEBUG] +++++++++++ Proc %d \t Fin de l'iteration %d \n", rank, etape);
#endif
    }// if (etape == 0)
    
    
    

    
    /*------   Attente de la reception non bloquante   ------*/      
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Attente de la reception.... \n", rank);
#endif
    MPI_Wait(&reqr, &status1);
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Fin de la reception\n", rank);
#endif
    
    /*------   Permutation de tableaux   ------*/      
    /* on swich les deux tableaux b2 et b_tmp, comme ca on utilisera 
     * les résultats envoyés par mon prédecesseur.
     * Ces résultats sont recus dans le tableau b_tmp
     */

    /* on ne switch */
    p_b2_old = p_b2;
    p_b2 = p_b_tmp;
    p_b_tmp = p_b2_old;
    
#ifdef _DEBUG_
    printf("\t [DEBUG] +++++++++++ Proc %d \t Fin de l'etape %d \n", rank, etape);    
#endif
    /* incrémente le nombre d'itérations */
    etape++;

  } // while (etape < p-1){

#ifdef _DEBUG_
  printf("\t [DEBUG] +++++++++++ Proc %d \t Fin de la fonction bodies_Compute_own_interaction \n", rank);
#endif
}
#else /* #ifdef _BODIES_SPLIT_DATA_ */





void 
bodies_Compute_own_interaction(bodies_t *FMB_RESTRICT p_b){

  bodies_ind_t i,j;
  bodies_ind_t n = bodies_Nb_bodies(p_b);

  body_t *FMB_RESTRICT p_body_i;
  body_t *FMB_RESTRICT p_body_j;

  COORDINATES_T pix, piy, piz;
  COORDINATES_T pjx, pjy, pjz;
  VALUES_T val_i;
  position_t *p_force_vector_j;
  COORDINATES_T fix, fiy, fiz;

  REAL_T eps_soft_square = FMB_Info.eps_soft_square;

  p_body_i = p_b->p_bodies;

  for(i=0; i<n-1; i++){
    pix = POSITION_GET_X(body_Get_p_position(p_body_i));
    piy = POSITION_GET_Y(body_Get_p_position(p_body_i));
    piz = POSITION_GET_Z(body_Get_p_position(p_body_i));
    val_i = body_Get_value(p_body_i);
    fix = POSITION_GET_X(body_Get_p_force_vector(p_body_i));
    fiy = POSITION_GET_Y(body_Get_p_force_vector(p_body_i));
    fiz = POSITION_GET_Z(body_Get_p_force_vector(p_body_i));

    p_body_j = p_body_i;

    for (j=i+1;	 j<n;	 j++){
      ++p_body_j;
      pjx = POSITION_GET_X(body_Get_p_position(p_body_j));
      pjy = POSITION_GET_Y(body_Get_p_position(p_body_j));
      pjz = POSITION_GET_Z(body_Get_p_position(p_body_j));
      p_force_vector_j = body_Get_p_force_vector(p_body_j);

      CHECK_IF_POSITION_ARE_TOO_CLOSE(pix, piy, piz, pjx, pjy, pjz); 
      DIRECT_COMPUTATION_MUTUAL_SOFT(pix, piy, piz,
				     pjx, pjy, pjz,
				     val_i,
				     body_Get_value(p_body_j),
				     fix, fiy, fiz,
				     POSITION_GET_X(p_force_vector_j), 
				     POSITION_GET_Y(p_force_vector_j), 
				     POSITION_GET_Z(p_force_vector_j), 
				     pot_i,
				     BODY_GET_POTENTIAL(p_body_j),
				     eps_soft_square);
    }

    position_Set_x(body_Get_p_force_vector(p_body_i), fix);
    position_Set_y(body_Get_p_force_vector(p_body_i), fiy);
    position_Set_z(body_Get_p_force_vector(p_body_i), fiz);
    ++p_body_i;
  }
}


#endif /* #ifdef _BODIES_SPLIT_DATA_ */
