#include "direct_computation.h"







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
  if (position_Are_too_close(pos_x_tgt, pos_y_tgt, pos_z_tgt, pos_x_src, pos_y_src, pos_z_src)){            \
    fprintf(f_output, "In file direct_computation.c: the two position are too close:\n");                   \
    pos_xyz_Display(pos_x_tgt, pos_y_tgt, pos_z_tgt, f_output, low);	\
    fprintf(f_output, "\t and \t");					\
    pos_xyz_Display(pos_x_src, pos_y_src, pos_z_src, f_output, low);	\
    fprintf(f_output, "\n");						\
    FMB_ERROR_BRIEF();							\
  }									\
}
#else 
#define CHECK_IF_POSITION_ARE_TOO_CLOSE(pos_x_tgt, pos_y_tgt, pos_z_tgt, pos_x_src, pos_y_src, pos_z_src) 
#endif 








/*********************************************************************************************
**********************************************************************************************

   bodies_Compute_own_interaction

**********************************************************************************************
*********************************************************************************************/


#ifdef _BODIES_SPLIT_DATA_

void 
bodies_Compute_own_interaction(bodies_t *FMB_RESTRICT p_b){

  bodies_ind_t i,j;
  bodies_ind_t n = bodies_Nb_bodies(p_b);

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

  p_px = p_b->p_pos_x;
  p_py = p_b->p_pos_y;
  p_pz = p_b->p_pos_z;

  p_val = bodies_Get_p_value(p_b, 0);
  p_fx = p_b->p_fx;
  p_fy = p_b->p_fy;
  p_fz = p_b->p_fz;

  for(i=0; i<n-1; i++){
    pix = p_px[i];
    piy = p_py[i];
    piz = p_pz[i];
    val_i = p_val[i];

    fix = p_fx[i];
    fiy = p_fy[i];
    fiz = p_fz[i];


    for (j=i+1;	 j<n;	 j++){
      pjx = p_px[j];
      pjy = p_py[j];
      pjz = p_pz[j];
      val_j = p_val[j];

      CHECK_IF_POSITION_ARE_TOO_CLOSE(pix, piy, piz, pjx, pjy, pjz);       
      DIRECT_COMPUTATION_MUTUAL_SOFT(pix, piy, piz,
				     pjx, pjy, pjz,
				     val_i,
				     val_j,
				     fix, fiy, fiz,
				     p_fx[j], p_fy[j], p_fz[j],
				     pot_i,
				     p_pot[j],
				     eps_soft_square);
    }

    p_fx[i] = fix;    
    p_fy[i] = fiy;
    p_fz[i] = fiz;   
  }
}


void 
bodies_Compute_own_interaction_par(bodies_t *FMB_RESTRICT bodies, bodies_t *FMB_RESTRICT current, int rank){
 bodies_ind_t i = 0,j = 0;
 bodies_ind_t n1 = bodies_Nb_bodies(bodies);
 bodies_ind_t n2 = bodies_Nb_bodies(current);

 FMB_CONST COORDINATES_T *FMB_RESTRICT p1_px;
 FMB_CONST COORDINATES_T *FMB_RESTRICT p1_py;
 FMB_CONST COORDINATES_T *FMB_RESTRICT p1_pz;
 FMB_CONST VALUES_T *FMB_RESTRICT p1_val;

 COORDINATES_T *FMB_RESTRICT p1_fx;
 COORDINATES_T *FMB_RESTRICT p1_fy;
 COORDINATES_T *FMB_RESTRICT p1_fz;


 COORDINATES_T fix, fiy, fiz;
 REAL_T eps_soft_square = FMB_Info.eps_soft_square;
 COORDINATES_T pix, piy, piz, pjx, pjy, pjz;
 VALUES_T val_i, val_j;

 
 FMB_CONST COORDINATES_T *FMB_RESTRICT p2_px;
 FMB_CONST COORDINATES_T *FMB_RESTRICT p2_py;
 FMB_CONST COORDINATES_T *FMB_RESTRICT p2_pz;
 FMB_CONST VALUES_T *FMB_RESTRICT p2_val;

 COORDINATES_T *FMB_RESTRICT p2_fx;
 COORDINATES_T *FMB_RESTRICT p2_fy;
 COORDINATES_T *FMB_RESTRICT p2_fz;


 p1_px = bodies->p_pos_x;
 p1_py = bodies->p_pos_y;
 p1_pz = bodies->p_pos_z;

 p1_fx = bodies->p_fx;
 p1_fy = bodies->p_fy;
 p1_fz = bodies->p_fz;

 p1_val = bodies_Get_p_value(bodies, 0);

 p2_px = current->p_pos_x;
 p2_py = current->p_pos_y;
 p2_pz = current->p_pos_z;

 p2_fx = current->p_fx;
 p2_fy = current->p_fy;
 p2_fz = current->p_fz;

 p2_val = bodies_Get_p_value(current, 0);

 for(i=0; i<n1; i=i+1){
   pix = p1_px[i];
   piy = p1_py[i];
   piz = p1_pz[i];

   val_i = p1_val[i];

   fix = p1_fx[i];
   fiy = p1_fy[i];
   fiz = p1_fz[i];
   
   for(j=0; j<n2; j=j+1){
   
     pjx = p2_px[j];
     pjy = p2_py[j];
     pjz = p2_pz[j];
     val_j = p2_val[j];
     
     CHECK_IF_POSITION_ARE_TOO_CLOSE(pix, piy, piz, pjx, pjy, pjz);
     DIRECT_COMPUTATION_MUTUAL_SOFT(pix, piy, piz,
				    pjx, pjy, pjz,
				    val_i,
				    val_j,
				    fix, fiy, fiz,
				    p2_fx[j], p2_fy[j], p2_fz[j],
				    pot_i,
				    p_pot[j],
				    eps_soft_square);
   }
   p1_fx[i] = fix;
   p1_fy[i] = fiy;
   p1_fz[i] = fiz;
 }

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










