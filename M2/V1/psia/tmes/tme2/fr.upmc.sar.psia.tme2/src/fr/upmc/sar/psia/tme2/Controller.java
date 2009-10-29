package fr.upmc.sar.psia.tme2;

import peersim.config.Configuration;
import peersim.core.Control;

public class Controller implements Control {

	private Statistique statistique;
	
	private static int   cptExpriences = 0;
	private static int   cptJours      = 0;
	private static int   nbExperiences = Configuration.getInt("simulation.experiments");
	private static int            step = Configuration.getInt("control.monmodule.step");
	private static int         nbJours = (Configuration.getInt("simulation.endtime") + 1) / step; // A Verifier pas tjs vrai

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

		if ((cptExpriences == nbExperiences) && (cptJours >= nbJours) ) {
			statistique.writeStat();
		}

		return false;
	}
}
