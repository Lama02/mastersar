package fr.upmc.sar.psia.tme2;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;

import peersim.core.CommonState;
import peersim.core.Control;

public class Controller implements Control {
	
	private static int cpt = 0;
	
	private BufferedWriter outMo;
	private BufferedWriter outMa;
	//private long time;

	public Controller(String prefix) {
		
	}

	@Override
	public boolean execute() {
		System.out.println("---------- cpt:" + cpt++);
		System.out.println("****** NbMort   : " + Grippe.nbMort   + "   ----    " + CommonState.getTime());
		System.out.println("****** NbMalalde: " + Grippe.nbMalade + "   ----    " + CommonState.getTime());
		
		try {
			outMo = new BufferedWriter(new FileWriter("./statistiqueMo.txt",true));
			outMo.write(CommonState.getTime() + "\t" + Grippe.nbMort + "\n");
			outMo.close();
			
			outMa = new BufferedWriter(new FileWriter("./statistiqueMa.txt",true));
			outMa.write(CommonState.getTime() + "\t" + Grippe.nbMalade + "\n");
			outMa.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return false;
	}
}
