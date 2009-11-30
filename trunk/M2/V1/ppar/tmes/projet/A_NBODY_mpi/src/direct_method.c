#include "direct_method.h"
#include "IO.h" 
#include "var_mpi.h"



/* Here are the initialization of the global variables: */
bodies_t bodies;
char *Direct_data_file;
bool Direct_are_data_bzipped2 = FALSE; 
position_t center;
COORDINATES_T half_side;

FMB_Info_t FMB_Info;


/* See definition in 'FMB.c'. */
void bunzip2_file(const char *filename);
void bzip2_file(const char *filename);












/*********************************************************************************************
**********************************************************************************************

   Direct_method_Init

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Init(){

  /* Checking: */
  if (f_output == NULL){
    FMB_error("'f_output' must be set.\n");
  }


  /************************************ eps_soft_square: **********************************************/
  fprintf(f_output, "Softening parameter: %.1e\n", FMB_Info.eps_soft); 
  FMB_Info.eps_soft_square = FMB_Info.eps_soft * FMB_Info.eps_soft;

  /* Clear 'center' and 'half_side': */
  position_Initialize(&center);
  half_side = (COORDINATES_T) 0.0;

}






/*********************************************************************************************
**********************************************************************************************

   Direct_method_Data

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Data(char *data_file){
  bodies_ind_t k;
  bodies_ind_t nb_bodies; 

  if (INFO_DISPLAY(2)){
    fprintf(f_output, "Opening data file \'%s\' for direct computation... \n", data_file); 
  }

  /* Initialize Input operations: */    
  FMB_IO_InitI(data_file);
  
  FMB_IO_Scan_header(&nb_bodies, &center, &half_side);

  if (INFO_DISPLAY(1)){
    fprintf(f_output, "Bodies number: ");
    fprintf(f_output, FORMAT_BODIES_IND_T, nb_bodies);
    fprintf(f_output, "\n"); 
    fflush(f_output);
  }


  bodies_Initialize(&bodies, nb_bodies);

  for (k=0; k<nb_bodies ;++k){
    /* We have to use 'bodies_Add()'! */
    body_t body_tmp;
    body_Initialize(&body_tmp);

    if (FMB_IO_Scan_body(&body_tmp) != 1){
      FMB_error("In Direct_method_Data(): FMB_IO_Scan_body() failed for body #%i\n", k);
    }

/*     if (k<100){ body_Display(&body_tmp, f_output); }  */
    
    bodies_Add(&bodies, &body_tmp);
  }

  bodies_ClearFP(&bodies);

  /* Terminate Input operations: */
  FMB_IO_TerminateI();

}




/*********************************************************************************************
 ********************************************************************************************
**********************************************************************************************

 Direct_method_Data_bodies

**********************************************************************************************
*********************************************************************************************/
 /* Same as Direct_method_Data() but we use the position and values
  * of all bodies stored in 'p_b' (instead of the bodies stored
  * in the file "data_file" in Direct_method_Data()). */
void Direct_method_Data_bodies(bodies_t *p_b){
  
  bodies_it_t it;

  bodies_Initialize(&bodies, bodies_Nb_bodies(p_b));

  for (bodies_it_Initialize(&it, p_b);
       bodies_it_Is_valid(&it);
       bodies_it_Go2Next(&it)){
    /* We have to use 'bodies_Add()'! */
    body_t body_tmp;
    bodies_it_Get_body(&it, &body_tmp);
    bodies_Add(&bodies, &body_tmp);
  }

  bodies_ClearFP(&bodies);

}

/*********************************************************************************************
**********************************************************************************************

   MPI_SEND_DATAS

**********************************************************************************************
*********************************************************************************************/
void mpi_isend(bodies_t * out, int to, MPI_Request *req_send){

  MPI_Isend(out->p_pos_x,
	    out->nb_bodies,
	    MPI_FLOAT,
	    to,
	    0,
	    MPI_COMM_WORLD,
	    &req_send[0]);
  MPI_Isend(out->p_pos_y,
	    out->nb_bodies,
	    MPI_FLOAT,
	    to,
	    0,
	    MPI_COMM_WORLD,
	    &req_send[1]);
  MPI_Isend(out->p_pos_z,
	    out->nb_bodies,
	    MPI_FLOAT,
	    to,
	    0,
	    MPI_COMM_WORLD,
	    &req_send[2]);
  MPI_Isend(out->p_values,
	    out->nb_bodies,
	    MPI_FLOAT,
	    to,
	    0,
	    MPI_COMM_WORLD,
	    &req_send[3]);  
}
void mpi_irecv(bodies_t * in, int from, MPI_Request *reqr){
  MPI_Irecv(in->p_pos_x,
	    in->nb_bodies,
	    MPI_FLOAT,
	    from,
	    0,
	    MPI_COMM_WORLD,
	    &reqr[0]);

  MPI_Irecv(in->p_pos_y,
	    in->nb_bodies,
	    MPI_FLOAT,
	    from,
	    0,
	    MPI_COMM_WORLD,
	    &reqr[1]);

  MPI_Irecv(in->p_pos_z,
	    in->nb_bodies,
	    MPI_FLOAT,
	    from,
	    0,
	    MPI_COMM_WORLD,
	    &reqr[2]);

  MPI_Irecv(in->p_values,
	    in->nb_bodies,
	    MPI_FLOAT,
	    from,
	    0,
	    MPI_COMM_WORLD,
	    &reqr[3]);

}
void mpi_iwait(MPI_Request *req){
  MPI_Status status;

  MPI_Wait(&req[0], &status);
  MPI_Wait(&req[1], &status);
  MPI_Wait(&req[2], &status);
  MPI_Wait(&req[3], &status);
}






/*********************************************************************************************
**********************************************************************************************

   Direct_method_Compute

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Compute(){

    /********************* Without reciprocity: *******************************************/
    /* bodies_Compute_own_interaction_no_mutual() is not implemented ... */

    /********************* With reciprocity: **********************************************/
    /* Compute the force and the potential: */
    bodies_Compute_own_interaction(&bodies);        


    /**************** Possible scaling with CONSTANT_INTERACTION_FACTOR: ********************/
    /* We can also use CONSTANT_INTERACTION_FACTOR only for the total potential energy ... */
#ifdef _USE_CONSTANT_INTERACTION_FACTOR_
    bodies_Scale_with_CONSTANT_INTERACTION_FACTOR(&bodies);
#endif /* #ifdef _USE_CONSTANT_INTERACTION_FACTOR_ */


}




void  Direct_method_Compute_Par(bodies_t * current, bodies_t * next){
  bodies_t * tmp;
  int succ = (rank+1)%mpi_p;
  int prev = (rank-1+mpi_p)%mpi_p;
  int pas = 1;
  MPI_Request req_send [4];
  MPI_Request req_recv [4];
  
  /*pas = 0*/
  mpi_isend(&bodies, succ, req_send);
  mpi_irecv(next, prev, req_recv);
  bodies_Compute_own_interaction(&bodies);
  mpi_iwait(req_send);
  mpi_iwait(req_recv);

  /*on switch les tableaux*/
  tmp = next;
  next = current;
  current = tmp;
  
  
  while(pas < mpi_p){

    mpi_isend(current, succ, req_send);
    mpi_irecv(next, prev, req_recv);
    
    bodies_Compute_own_interaction_par(&bodies,current,rank);
    mpi_iwait(req_send);
    mpi_iwait(req_recv);
    
    /*on switch les tableaux*/
    tmp = next;
    next = current;
    current = tmp;
    
    pas++;
  }

#ifdef _USE_CONSTANT_INTERACTION_FACTOR_
    bodies_Scale_with_CONSTANT_INTERACTION_FACTOR(&bodies);
#endif 


}





/*********************************************************************************************
**********************************************************************************************
************************* Move of the bodies: ************************************************

   Direct_method_Move : Leapfrog integrator ( Kick Drift Kick )  

**********************************************************************************************
*********************************************************************************************/

void KnD_Direct_method_Move(REAL_T dt ){
  /**** Kick N Drift ***/
	bodies_it_t it;
 for (bodies_it_Initialize(&it, &bodies);
      bodies_it_Is_valid(&it);
      bodies_it_Go2Next(&it)){
   bodies_Kick_Move(&it,dt);
   bodies_Drift_Move(&it,dt); 
 }
}

void K_Direct_method_Move(REAL_T dt ){
  /************************* Move of the bodies: ******************************************/
  bodies_it_t it;
  for (bodies_it_Initialize(&it, &bodies);
       bodies_it_Is_valid(&it);
       bodies_it_Go2Next(&it)){
    bodies_Kick_Move(&it,dt);
  }
}










/*********************************************************************************************
**********************************************************************************************

   Direct_method_Terminate

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Terminate(){

  bodies_Free(&bodies);

  if (Direct_are_data_bzipped2){
    /* We recompress the data file: */
    bzip2_file(Direct_data_file);
  }
  FMB_free(Direct_data_file);

}



























/*********************************************************************************************
**********************************************************************************************

   sum

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Sum(char *results_file,
		       unsigned long step_number_value,
		       bodies_t *p_bodies, 
		       VALUES_T total_potential_energy){

  FILE *f_results;
  position_t force_sum;
  bodies_it_t it;

  f_results = f_output;

  position_Initialize(&force_sum);
  for (bodies_it_Initialize(&it, p_bodies);
       bodies_it_Is_valid(&it);
       bodies_it_Go2Next(&it)){ 
    position_Set_x(&force_sum, position_Get_x(&force_sum) + bodies_it_Get_fx(&it));
    position_Set_y(&force_sum, position_Get_y(&force_sum) + bodies_it_Get_fy(&it));
    position_Set_z(&force_sum, position_Get_z(&force_sum) + bodies_it_Get_fz(&it));
  }
  fprintf(f_results, "Sum (force): ");
  position_Display(&force_sum, f_results, high);

  fprintf(f_results, "\n");

}








/*********************************************************************************************
**********************************************************************************************

   save 

**********************************************************************************************
*********************************************************************************************/
void Direct_method_Dump_bodies(char *results_filename,
			       unsigned long step_number_value,
			       bodies_t *p_bodies){
  bodies_it_t it;

  /* Initialize Ouput operations: */    
  FMB_IO_InitO(results_filename);
  
  if (FMB_IO_Info.output_format != NEMO_format){
    
    /********** FMB file format: **********/
    if (FMB_IO_Info.output_format == FMB_binary_format){
      FMB_error("Unable to write the 'header' for FMB_binary_format in Direct_method_Dump_bodies(). \n");
    }
    FMB_IO_Print_header(step_number_value, FALSE /* only_position_and_value */,
			bodies_Nb_bodies(p_bodies), &center, half_side);
    
    for (bodies_it_Initialize(&it, p_bodies);
	 bodies_it_Is_valid(&it);
	 bodies_it_Go2Next(&it)){ 
      
      FMB_IO_Print_body_from_bodies_it(&it, FALSE /* only_position_and_value */);
    } /* for */
    
  } /* if (FMB_IO_Info.output_format != NEMO_format) */
  else {
    /********** NEMO file format: **********/
    FMB_IO_Print_all_bodies_from_bodies_t(p_bodies);
  } /* else (FMB_IO_Info.output_format != NEMO_format) */
  
  /* Terminate Output operations: */    
  FMB_IO_TerminateO();

}














































