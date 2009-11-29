#!//usr/bin/ruby



### exemple d execution 
# ruby generate_stats_fils.rb plumer2048bare2048_1MPI-par-core_1MPI-par-noeud \
# ../logs/plumer2048bare2048_2proc_1machine ../logs/plumer2048bare2048_2proc_2machine \
# ../logs/plumer2048bare2048_4proc_2machine ../logs/plumer2048bare2048_4proc_4machine \
# ../logs/plumer2048bare2048_8proc_4machine ../logs/plumer2048bare2048_8proc_8machine \
# ../logs/plumer2048bare2048_16proc_8machine ../logs/plumer2048bare2048_16proc_16machine


## ce tableau contiendra des tableau de step.
@@tab_process = []








#########################################################################################
# GET_TEMPS_TOTOAL
#########################################################################################
# calcule la moyenne du temps utilise pour faire tout le calcul
def get_temps_total
    nb_process = @@tab_process.size
    temps_total = 0
    for proc in @@tab_process
    	## proc correspond au tableau tab_step de chaque proc
	for step in proc
	    temps_total += step["Computation_time"]
	end
    end
    # on retourne la moyenne du temps
    (temps_total / nb_process )
end




#########################################################################################
# LA FONCTION REMPLIR_STRUCT
#########################################################################################
def remplir_struct(tab)
    start = 0 ## je suis pas encore arrive aux lignes contenant les stats
    for element in tab.split(',') 
    	element
    	## element est de la forme "process : 9" ou de la forme "  Interactions_computed : 4192256"
	couple = element.split(':')
    	## couple est de la forme [" Gflop/s ", " 0.016"]
    	first_element = couple.first.strip
    	# first_element est de la forme "Gflop/s"
    	second_element = couple.last.strip
    	# second_element est de la forme "0.016"

	## on a affair à quel process ?
    	if first_element == "process" then
	   start = 1
      	   ## on initialise la valeur de la 
      	   ## variable process
      	   process = second_element.strip.to_i
     
	## on a affair à quel step ?
    	elsif first_element == "Step_number" then
      	      ## on initialise la valeur de la 
      	      ## variable step
      	      step = second_element.strip.to_i
    	elsif start == 1
	      tab_step = @@tab_process.at(process)
      	      tab_step = [] if tab_step == nil
      	      tab_stats = tab_step[step]
	      tab_stats = {} if tab_stats == nil
	      tab_stats[first_element] = second_element.strip.to_f
      	      tab_step[step] = tab_stats 
      	      @@tab_process[process]=tab_step
     	end
    end
end







#########################################################################################
# LA FONCTION MAIN
#########################################################################################
## la fonction main
def main
    to_save = ""
    cpt = 1
    file_dest_name = ARGV[0]   
    file_save = File.open(file_dest_name, "w")
    while (file_name = ARGV[cpt])
    	  ## le nom du ficher à ouvrir est passé 
    	  ## argument
    	  #file_name = ARGV[0]
    	  file = File.new(file_name, "r")
    	  while (line = file.gets) do
          	remplir_struct(line)
    	  end
    	  file.close
    
	  nb_process = @@tab_process.size
   	  nb_steps = @@tab_process[0].size
   	  temps = get_temps_total
    	  p  nb_process.to_s + " "+ temps.to_s
	  if ((cpt.modulo 2) == 0) then 
	     to_save += temps.to_s + "\n"
	     # je save le fichier
	     file_save.write(to_save)
	     # je remets to_save a ""
	     to_save = ""
	  else
	     to_save += nb_process.to_s + " " + temps.to_s + " "
	
	  end
	  cpt += 1
     end
     file_save.close

	  
end









#########################################################################################
# ENTREE DU PROGRAMME
#########################################################################################

main