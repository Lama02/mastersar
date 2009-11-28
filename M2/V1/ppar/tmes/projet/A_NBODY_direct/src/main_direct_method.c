/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */
#include <sys/stat.h>
#include <mpi.h>

#include "direct_method.h"
#include "IO.h" 


/* For FMB_Info.save: */
#define RESULTS_DIR "/tmp/NBODY_direct_results_rachid_cote/"
#define RESULTS_FILE "results_"


/*** For timers: ***/
#include <sys/time.h>

/*********************************************************************************************
**********************************************************************************************/

/*variables MPI*/
MPI_Status status;
int rank = 0;
int mpi_p = 0;
int mpi_tag = 0;


/*variable de parallesisation*/
long nb_bodies_local; //nombre de corps local au noeud
long nb_bodies_total; //nombre de corps dans le systeme
REAL_T dt;           
bodies_t p_b1;       //dtructure qui servira de tampon pour les emission et les reception
bodies_t p_b2;       //structure qui servira de tampon pour les emission et les reception
bodies_t *current_b; //pointeur vers les structures tampon
bodies_t *next_b;    //pointeur vers les structures tampon
REAL_T tstart ,tend , tnow ; 

/*********************************************************************************************
**********************************************************************************************/
void initialize_node(){
  /*calcul du nombre de corps par noeud*/
  if (rank == 0)
    nb_bodies_local = bodies.size_allocated/mpi_p;

  
  nb_bodies_total = bodies.nb_bodies;

  /*on diffuse le nombre de corps sue devra gerer chaque noeud*/
  MPI_Bcast(&nb_bodies_local,
	    1,
	    MPI_LONG,
	    0,
	    MPI_COMM_WORLD);
  
  /*on initialise les structure tampon*/
  bodies_Initialize(&p_b1,nb_bodies_local);
  bodies_Initialize(&p_b2,nb_bodies_local);
  current_b = &p_b1;
  next_b = &p_b2;
  
  /*on initialise la stucture bodies sauf pour le proc 0 qui l'a deja faite*/
  if(rank != 0){
    bodies_Initialize(&bodies,nb_bodies_local);
    bodies.size_allocated = nb_bodies_local;    
  }
}
void scatter_to_nodes(){
  //repartition des pos_x
  MPI_Scatter(bodies.p_pos_x,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_pos_x,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);
		
  //repartition des pos_y
  MPI_Scatter(bodies.p_pos_y,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_pos_y,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);
    
  //repartition des pos_z
  MPI_Scatter(bodies.p_pos_z,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_pos_z,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);
    
  //repartition des fx
  MPI_Scatter(bodies.p_fx,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_fx,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);
		
  //repartition des fy
  MPI_Scatter(bodies.p_fy,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_fy,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);
    
  //repartition des fz
  MPI_Scatter(bodies.p_fz,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_fz,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);

  //repartition des values
  MPI_Scatter(bodies.p_values,
	      nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_values,
	      nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);    

  //repartition des spped_vector
  MPI_Scatter(bodies.p_speed_vectors,
	      3*nb_bodies_local,
	      MPI_FLOAT,
	      bodies.p_speed_vectors,
	      3*nb_bodies_local,
	      MPI_FLOAT,
	      0,
	      MPI_COMM_WORLD);  
    
  //envoi de la date de fin
  MPI_Bcast(&tend,
	    1,
	    MPI_FLOAT,
	    0,
	    MPI_COMM_WORLD);


  //envoi du pas de temps
  dt = FMB_Info.dt;
  MPI_Bcast(&dt,
	    1,
	    MPI_FLOAT,
	    0,
	    MPI_COMM_WORLD);
  bodies.nb_bodies = nb_bodies_local;
}

void gather_to_root(){
  //recuperation des pos_x
  MPI_Gather(bodies.p_pos_x,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_pos_x,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des pos_y
  MPI_Gather(bodies.p_pos_y,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_pos_y,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des pos_z
  MPI_Gather(bodies.p_pos_z,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_pos_z,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des fx
  MPI_Gather(bodies.p_fx,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fx,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des fy
  MPI_Gather(bodies.p_fy,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fy,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des fz
  MPI_Gather(bodies.p_fz,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fz,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des values
  MPI_Gather(bodies.p_values,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_values,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);    
      
  //recuperation des spped_vector
  MPI_Gather(bodies.p_speed_vectors,
	     3*nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_speed_vectors,
	     3*nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
}
void get_forces(){
  //recuperation des fx
  MPI_Gather(bodies.p_fx,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fx,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des fy
  MPI_Gather(bodies.p_fy,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fy,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
      
  //recuperation des fz
  MPI_Gather(bodies.p_fz,
	     nb_bodies_local,
	     MPI_FLOAT,
	     bodies.p_fz,
	     nb_bodies_local,
	     MPI_FLOAT,
	     0,
	     MPI_COMM_WORLD);
}

/*********************************************************************************************
**********************************************************************************************/

double my_gettimeofday(){
  struct timeval tmp_time;
  gettimeofday(&tmp_time, NULL);
  return tmp_time.tv_sec + (tmp_time.tv_usec * 1.0e-6L);
}



/* See definition at the end of this file. */
int parse_command(int argc, 
		  char **argv,
		  char **p_data_file,
		  char **p_results_file);



/*********************************************************************************************
**********************************************************************************************

   MAIN 

   **********************************************************************************************
   *********************************************************************************************/

int main(int argc, char **argv){


  long nb_steps = 0;
  tstart = 0 ; 
  tnow = tstart ; 
  tend = 0.001 ; 

  char *data_file = NULL;
  char *results_file = NULL;
  VALUES_T total_potential_energy = 0.0;

  /* Timers: */
  double t_start = 0.0, t_end = 0.0;


  /********************************* Options on command line: ***************************************/
  f_output = stdout; /* by default */
  parse_command(argc, argv, &data_file, &results_file);
  



  /******************************** Files and FILE* : ***************************************/
  if (INFO_DISPLAY(1)){
    fprintf(f_output, 
	    "*** Compute own interactions of the box defined in \"%s\" ***.\n", 
	    data_file);
  }

  Direct_method_Init();


  /***************************** Bodies'positions and masses initialization: ****************/
  Direct_method_Data(data_file);

  tend=FMB_Info.tend;
  if (INFO_DISPLAY(1)){ 
    fprintf(f_output, "Start Time : %lf \t End Time : %lf \t dt : %lf \n",tstart, tend, FMB_Info.dt);
    fprintf(f_output, "Number of steps: %lu\n", (unsigned long) ((tend-tstart)/FMB_Info.dt));
  }
 
  /***************************** Initialisation parallele  **************** ****************/

  /*initialisation MPI*/
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&mpi_p);
  
  if(rank!=0)
    Direct_method_Init();
  
  initialize_node();
  scatter_to_nodes();
  
  
  
  



  /******************************************************************************************/
  /********************************** Start of the simulation: ******************************/
  /******************************************************************************************/

  while ( tnow-FMB_Info.dt < tend ) { 

    /********************* Direct method computation: ************************************/
    /*********************Direct metho Move : K-D-K **************************************/ 
    if(tnow!=0) {

      KnD_Direct_method_Move(FMB_Info.dt ); 
      
    
      /***** Clear the forces and the potential of the bodies for the next time step: ********/
      bodies_ClearFP(&bodies);
    }

    /* Start timer: */
    t_start = my_gettimeofday();

    /* Computation: */
    Direct_method_Compute();
   
    /* End timer: */
    t_end = my_gettimeofday();

    if (tnow !=0)K_Direct_method_Move(FMB_Info.dt);




    /****************** Save & display the total time used for this step: *******************/

    if (INFO_DISPLAY(1)){
      unsigned long long nb_int = NB_OWN_INT(bodies_Nb_bodies(&bodies));
       
      fprintf(f_output, "\n#######################################################################\n");
      fprintf(f_output, "Time now ( Step number) : %lf (%ld) , process %d\n",tnow,nb_steps,rank );
      fprintf(f_output, "Computation time = %f seconds\n", t_end - t_start);

      fprintf(f_output, "Interactions computed: %llu\n", nb_int);
      fprintf(f_output, "  Nb interactions / second: %.3f\n", ((double) nb_int) / (t_end - t_start));

      fprintf(f_output, "  Gflop/s = %.3f (11.5 flop with mutual) \n",
	      ((((double) nb_int) / (t_end - t_start)) * 11.5 /* 23 flops divided by 2 since mutual */) / (1000000000.0));

      
    }


    /************************* Save the positions and the forces: ***************************/
    
    /*on recupere les données de chaque noeud du systeme*/
    if(FMB_Info.save){
      gather_to_root();
    }
    if (FMB_Info.save && rank == 0){    

      if (results_file == NULL){
	/* The 'results' filename has not been set yet: */
#define TMP_STRING_LENGTH 10
	char step_number_string[TMP_STRING_LENGTH];
	int  results_file_length = 0;
	
	/* Find the relative filename in 'data_file': */
	char *rel_data_file = strrchr(data_file, '/') + 1 ; /* find last '/' and go to the next character */
	
	results_file_length = strlen(RESULTS_DIR) + 
	  strlen(RESULTS_FILE) + 
	  strlen(rel_data_file) +  
	  TMP_STRING_LENGTH + 
	  1 /* for '\0' */ ; 
	
	results_file = (char *) FMB_malloc_with_check(results_file_length * sizeof(char));
	
	strncpy(step_number_string, "", TMP_STRING_LENGTH);
	sprintf(step_number_string, "_%lu", nb_steps ); 
	
	strncpy(results_file, "", results_file_length);
	strcpy(results_file, RESULTS_DIR); 
	strcat(results_file, RESULTS_FILE); 
	strcat(results_file, rel_data_file); 
	strcat(results_file, step_number_string); 
#undef TMP_STRING_LENGTH
      }
      
      /* Create directory RESULTS_DIR: */
      {	struct stat filestat;
	if (stat (RESULTS_DIR, &filestat) != 0) {
	  /* The directory RESULTS_DIR does not exist, we create it: */
	  mkdir(RESULTS_DIR, 0700); 
	}
      }

      Direct_method_Dump_bodies(results_file, nb_steps, &bodies);

      FMB_free(results_file);
      results_file= NULL ; 
      
    }

    /************************** Sum of forces and potential: ***************************/
    if (FMB_Info.sum){
      //on verifie si l'option save n'est pas deja active
      if(!FMB_Info.save){
	get_forces();
      }
      //on met le nombre de corps à n
      if(rank == 0){
	bodies.nb_bodies = nb_bodies_total;
	Direct_method_Sum(NULL, nb_steps, &bodies, total_potential_energy);     
      }            
      //on remet le nombre de corps à n/p
      if(rank == 0){
	bodies.nb_bodies = nb_bodies_local;
      }
    }


    tnow+=FMB_Info.dt ; 
    nb_steps ++ ; 

  }  /* while ( tnow-FMB_Info.dt <= tend )  */

  
  MPI_Barrier(MPI_COMM_WORLD);
  fprintf(f_output, "*#*#1 process %d sort du while\n",rank);
   
  /******************************************************************************************/
  /********************************** End of the simulation: ********************************/
  /******************************************************************************************/

  MPI_Barrier(MPI_COMM_WORLD);
  Direct_method_Terminate();
  fprintf(f_output, "*#*#2 process %d direct terminate\n",rank);


  MPI_Barrier(MPI_COMM_WORLD);
  fprintf(f_output, "*#*#3 process %d se termine correctement\n",rank);    
  MPI_Finalize();
  /********************** Close FILE* and free memory before exiting: ***********************/
  if (argc == 3)
    if (fclose(f_output) == EOF)
      perror("fclose(f_output)");
  
  FMB_free(data_file);
  fprintf(f_output, "*#*#4 le programme se termine corectement; %d\n",rank);    
  /****************************************** EXIT ******************************************/
  exit(EXIT_SUCCESS);
}















/*********************************************************************************************
**********************************************************************************************

   usage

   **********************************************************************************************
   *********************************************************************************************/

void usage(){
  char mes[300] = "";
  
  sprintf(mes, "Usage : a.out [-h] %s [-o output_filename] --in[r]=data_filename %s \n"
	  , "[--soft value]"
	  , ""
	  );
  
  fprintf(stderr, "%s", mes);


  fprintf(stderr, "\nDescription of the short options:\n"); 
  /*   fprintf(stderr, "\t -v \t\t\t Display the version.\n"); */
  fprintf(stderr, "\t -h \t\t\t Display this message.\n"); 
  fprintf(stderr, "\t -i 'level' \t\t Info display level (0, 1 or 2).\n");
  fprintf(stderr, "\t -o 'output_filename' \t Otherwise stdout.\n");


  fprintf(stderr, "\nDescription of the long options:\n");
  fprintf(stderr, "\t --in='filename' \t Input data filename.\n");

  fprintf(stderr, "\t --save \t\t Save position, mass, force and/or potential of all particles.\n");

  /* Unused in this code: */
  /*   fprintf(stderr, "\t --out='filename' \t Output data filename for '--save' option.\n"); */

  fprintf(stderr, "\t --sum  \t\t Compute and display the sum of the forces and/or potential over all particles.\n");
  fprintf(stderr, "\t --soft='value' \t Softening parameter.\n");
  fprintf(stderr, "\t --dt='value' \t\t Leapfrog integration timestep \n");
  fprintf(stderr, "\t --tend='value' \t Time to stop integration \n");

  /* We use only NEMO file format in this code: */
  /*   fprintf(stderr, "\t --it='value' \t\t input  data format ('fma' for FMB ASCII, 'fmb' for FMB binary, 'nemo').\n"); */
  /*   fprintf(stderr, "\t --ot='value' \t\t output data format ('fma' for FMB ASCII, 'fmah' for FMB ASCII human readable, 'fmb' for FMB binary, 'nemo').\n"); */

  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}










/*********************************************************************************************
**********************************************************************************************

   parse_command

   **********************************************************************************************
   *********************************************************************************************/


/* Long option codes for 'val' field of struct option. 
 * Ascii codes 65 -> 90 ('A'->'Z') and 97 -> 122 ('a'->'z') 
 * are reserved for short options */
/* Same code as in main.c: */
#define LONGOPT_CODE_SOFT    14
#define LONGOPT_CODE_SAVE    24
#define LONGOPT_CODE_SUM     25
#define LONGOPT_CODE_IT 34
#define LONGOPT_CODE_OT 35
#define LONGOPT_CODE_IN 41
#define LONGOPT_CODE_OUT 43
#define LONGOPT_CODE_DT 49
#define LONGOPT_CODE_TEND 48 

int parse_command(int argc, 
		  char **argv,
		  char **p_data_file,
		  char **p_results_file){
  char options[]="hi:o:";
  int curr_opt;
  /*   opterr = 0; */

  /* Default values: */
  FMB_Info.dt = 0.001;
  FMB_Info.tend = 0.001 ; 
  FMB_Info.eps_soft = 0.0;

  
  struct option longopts[] = {
    {"soft",
     required_argument,
     NULL, 
     LONGOPT_CODE_SOFT},
    {"dt",
     required_argument,
     NULL, 
     LONGOPT_CODE_DT},
    {"tend",
     required_argument,
     NULL, 
     LONGOPT_CODE_TEND},
    {"save",
     no_argument,
     NULL,
     LONGOPT_CODE_SAVE},
    {"sum",
     no_argument,
     NULL, 
     LONGOPT_CODE_SUM},
    {"it",
     required_argument,
     NULL, 
     LONGOPT_CODE_IT},
    {"ot",
     required_argument,
     NULL, 
     LONGOPT_CODE_OT},
    {"in",
     required_argument,
     NULL, 
     LONGOPT_CODE_IN},
    {"out",
     required_argument,
     NULL, 
     LONGOPT_CODE_OUT},
    {0}}; /* last element of the array  */
  

  
  if (argc == 1){
    usage();
  }


  /* Default values: see direct_*/
  FMB_Info.eps_soft = 0.0;

  
  curr_opt=getopt_long(argc, argv, options, longopts, NULL);
  while(curr_opt != (int) EOF){

    switch(curr_opt){
    case 'h' : 
      usage();
      break;
    case 'i':
      FMB_IO_Info.info_display_level = atoi(optarg);
      if (FMB_IO_Info.info_display_level != 0 && 
	  FMB_IO_Info.info_display_level != 1 &&
	  FMB_IO_Info.info_display_level != 2 &&
	  FMB_IO_Info.info_display_level != 3){
	FMB_error("Wrong FMB_IO_Info.info_display_level value.\n");
      }
      break;
    case 'o':
      if ((f_output = fopen(optarg, "w")) == NULL){
	perror("fopen(\'output_filename\', \"w\")");
      }	  
      break;
    case '?' : 
      usage();
      break;

      
    case LONGOPT_CODE_SOFT:
      FMB_Info.eps_soft = (REAL_T) atof(optarg);
      break;
    case LONGOPT_CODE_DT:
      FMB_Info.dt =(REAL_T) atof(optarg);
      break;
    case LONGOPT_CODE_TEND:
      FMB_Info.tend =(REAL_T) atof(optarg) ; 
      break;
    case LONGOPT_CODE_SAVE:
      FMB_Info.save = TRUE;
      break;
    case LONGOPT_CODE_SUM:
      FMB_Info.sum  = TRUE;
      break;      
    case LONGOPT_CODE_IT:
      if (strcmp(optarg, "fma") == 0){
	FMB_IO_Info.input_format = FMB_ASCII_format;
      }
      else {
	if (strcmp(optarg, "fmah") == 0){
	  FMB_error("FMB_ASCII_human_format is only for \"output format\", not for \"input format\".\n");
	} 
	else {
	  if (strcmp(optarg, "fmb") == 0){
	    FMB_IO_Info.input_format = FMB_binary_format;
	  }
	  else {
	    if ((strcmp(optarg, "nemo") == 0) || (strcmp(optarg, "NEMO"))){
	      FMB_IO_Info.input_format = NEMO_format;
	    }
	    else {
	      FMB_error("Unknow format for --it option!\n");	    
	    }
	  }
	}
      }
      FMB_IO_Info.input_format_from_cmd_line = TRUE;
      break;
    case LONGOPT_CODE_OT:
      if (strcmp(optarg, "fma") == 0){
	FMB_IO_Info.output_format = FMB_ASCII_format;
      }
      else {
	if (strcmp(optarg, "fmah") == 0){
	  FMB_IO_Info.output_format = FMB_ASCII_human_format;
	}
	else {
	  if (strcmp(optarg, "fmb") == 0){
	    FMB_IO_Info.output_format = FMB_binary_format;
	  }
	  else {
	    if ((strcmp(optarg, "nemo") == 0) || (strcmp(optarg, "NEMO"))){
	      FMB_IO_Info.output_format = NEMO_format;
	    }
	    else {
	      FMB_error("Unknow format for --it option!\n");	    
	    }
	  }
	}
      }
      FMB_IO_Info.output_format_from_cmd_line = TRUE;
      break;
    case LONGOPT_CODE_IN:
      if (*p_data_file != NULL){ FMB_ERROR_BRIEF(); }
      *p_data_file = (char *) FMB_malloc_with_check((strlen(optarg) + 1 /* for '\0' */) * sizeof(char));
      strcpy(*p_data_file, optarg); 
      break;
    case LONGOPT_CODE_OUT:
      if (*p_results_file != NULL){ FMB_ERROR_BRIEF(); }
      *p_results_file = (char *) FMB_malloc_with_check((strlen(optarg) + 1 /* for '\0' */) * sizeof(char));
      strcpy(*p_results_file, optarg); 
      break;
    } /* switch */
    curr_opt=getopt_long(argc, argv, options, longopts, NULL);
  }

  /* Check that an input data filename has been provided: */
  if (*p_data_file == NULL){
    FMB_error("No 'input data filename' provided.\n");
  }

  return 0;
}




















