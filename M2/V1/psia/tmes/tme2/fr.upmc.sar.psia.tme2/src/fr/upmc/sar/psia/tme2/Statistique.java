package fr.upmc.sar.psia.tme2;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import peersim.config.Configuration;

public class Statistique {
	private static Statistique instance;

	private BufferedWriter outMo;
	private BufferedWriter outMa;

	private static int  nbExperiences = Configuration.getInt("simulation.experiments");
	private static int           step = Configuration.getInt("control.monmodule.step");
	private static int        nbJours = (Configuration.getInt("simulation.endtime") + 1) / step;

	private static List<Integer> statMorts    = new ArrayList<Integer>();
	private static List<Integer> statMalades  = new ArrayList<Integer>();


	private Statistique() {}

	public final synchronized static Statistique getInstance() {
		if (instance == null) {
			instance = new Statistique();
			for(int i = 0; i < nbJours; i++) {
				statMalades.add(0);
				statMorts.add(0);
			}
		}
		return instance;
	}

	public void saveNbMalades(int day, Integer nbMalades){
		statMalades.set(day, nbMalades + statMalades.get(day));
	}

	public void saveNbMorts(int day, Integer nbMorts){
		statMorts.set(day, nbMorts + statMorts.get(day));
	}

	public void writeStat() {
		try {

			outMo = new BufferedWriter(new FileWriter("./statistiqueMo.txt"));
			outMo.close();
			outMa = new BufferedWriter(new FileWriter("./statistiqueMa.txt"));
			outMa.close();
			
			outMo = new BufferedWriter(new FileWriter("./statistiqueMo.txt",true));
			outMa = new BufferedWriter(new FileWriter("./statistiqueMa.txt",true));
			for (int i = 0; i < nbJours; i++) {
				outMo.write(i * step + "\t" + (statMorts.get(i) / nbExperiences)  + "\n");
				outMa.write(i * step + "\t" + (statMalades.get(i) / nbExperiences) + "\n");
			}
			outMo.close();
			outMa.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
