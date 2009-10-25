package fr.upmc.sar.psia.tme2;

import peersim.core.CommonState;
import peersim.core.Control;

public class Controller implements Control {

	public Controller(String prefix) {
		
	}
	
	@Override
	public boolean execute() {
		
		System.out.println("****** NbMort   : " + Grippe.nbMort   + "   ----    " + CommonState.getTime());
		System.out.println("****** NbMalalde: " + Grippe.nbMalade + "   ----    " + CommonState.getTime());
		
		return false;
	}

}
