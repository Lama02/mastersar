#!//usr/bin/ruby






## ce tableau contiendra des tableau de step.
@@tab_process = []
## le nombre de pas
@@nb_step = 100 







#########################################################################################
# GET_TEMPS_TOTOAL
#########################################################################################
# calcule la moyenne du temps utilise pour faire tout le calcul
def get_temps_total

end




#########################################################################################
# LA FONCTION REMPLIR_STRUCT
#########################################################################################
def remplir_struct(tab)
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
      	   ## on initialise la valeur de la 
      	   ## variable process
      	   process = second_element.strip.to_i
     
	## on a affair à quel step ?
    	elsif first_element == "Step_number" then
      	      ## on initialise la valeur de la 
      	      ## variable step
      	      step = second_element.strip.to_i
    	else
	      tab_step = @@tab_process.at(process)
      	      tab_step = [] if tab_step == nil
      	      tab_stats = tab_step[step]
	      tab_stats = {} if tab_stats == nil
	      tab_stats[first_element] = second_element.strip.to_f
      	      tab_step[step] = tab_stats 
      	      @@tab_process[process]=tab_step
     	end
    end
		#p "#########################################"
		#p "###### step " + step.to_s + " process " + process.to_s
		#p tab_step[step]
		#p "+++++++++++"
		#p @@tab_process[process][step]

end







#########################################################################################
# LA FONCTION MAIN
#########################################################################################
## la fonction main
def main
    ## le nom du ficher à ouvrir est passé 
    ## argument
    file_name = ARGV[0]
    cpt = 0
    file = File.new(file_name, "r")
    while (line = file.gets) do
        remplir_struct(line) if cpt > 10
	cpt += 1
    end
    file.close
    p "****************************"
    p "****************************"
    p "process : 0, Step_number : 84 ,Computation_time :"
    p @@tab_process[0][84]["Computation_time"]
end








#########################################################################################
# ENTREE DU PROGRAMME
#########################################################################################

main