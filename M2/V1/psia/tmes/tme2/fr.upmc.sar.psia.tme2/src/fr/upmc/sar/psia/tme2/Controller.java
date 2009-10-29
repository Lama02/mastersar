package fr.upmc.sar.psia.tme2;

import peersim.config.Configuration;
import peersim.core.Control;

public class Controller implements Control {

	private Statistique statistique;
	
	private static int   cptExpriences = 0;
	private static int   cptJours      = 0;
	private static int   nbExperiences = Configuration.getInt("simulation.experiments");
	private static int            step = Configuration.getInt("control.monmodule.step");
	private static int         nbJours = (Configuration.getInt("simulation.endtime") + 1) / step; //TODO A Verifier pas tjs vrai

	public Controller(String prefix) {
		statistique = Statistique.getInstance();
		cptExpriences++;
		cptJours = 0;
	}

	@Override
	public boolean execute() {
		statistique.saveNbMalades(cptJours, Grippe.nbMalade);
		statistique.saveNbMorts(cptJours, Grippe.nbMort);
		cptJours++;
		if (((cptExpriences %10 ) == 0) && (cptJours >= nbJours) ) {
			// on calcule la moyennes des malades par jour
			statistique.saveMoyMalades();
			// on calcule la moyennes des morts par jour
			statistique.saveMoyMorts();

			if (Configuration.getBoolean("mode_vacc") ){
				// si le mode vaccination est active				
				// on calcule le max des malades par poucentage de vaccination
				statistique.saveMaxMalades();
				// on calcule le max des morts par poucentage de vaccination
				statistique.saveMaxMorts();

				Statistique.taux_vacc += 0.01;
				
				// TODO reinitialiser les deux tableaux 
				statistique.init();
				if (cptExpriences == nbExperiences ){
					// si on est arrive a la derniere iteration 
					statistique.writeStatVaccinations();
				}
			}else{
				// le mode vaccination n'est pas active
				// on sauvegarde les stats dans le fichier
				statistique.writeStat();
			}
			
		}

		return false;
	}
}
