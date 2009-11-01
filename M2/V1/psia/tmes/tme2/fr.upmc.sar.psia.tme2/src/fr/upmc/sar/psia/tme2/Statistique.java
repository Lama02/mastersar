package fr.upmc.sar.psia.tme2;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import peersim.config.Configuration;

public class Statistique {
	private static Statistique instance;
	
	public static double taux_vacc = 0.0;
	private BufferedWriter outMo;
	private BufferedWriter outMa;

	private static int  nbExperiences = Configuration.getInt("simulation.experiments");
	private static int           step = Configuration.getInt("control.monmodule.step");
	private static int        nbJours = (Configuration.getInt("simulation.endtime") + 1) / step;

	private static List<Double> statMorts    = new ArrayList<Double>();
	private static List<Double> statMalades  = new ArrayList<Double>();

	private static List<Double> statMaxMorts    = new ArrayList<Double>();
	private static List<Double> statMaxMalades  = new ArrayList<Double>();


	private Statistique() {}

	/**
	 * singleton 
	 * @return L'instance de la classe Statistique
	 */
	public final synchronized static Statistique getInstance() {
		if (instance == null) {
			instance = new Statistique();
			for(int i = 0; i < nbJours; i++) {
				statMalades.add(0.0);
				statMorts.add(0.0);
			}
		}
		return instance;
	}

	/**
	 * Initialiser la liste contenant le nombre de 
	 * morts/malades par unitŽ de temps (par jour par exemple)
	 */
	public void init(){
		for(int i = 0; i < nbJours; i++) {
			statMalades.set(i, 0.0);
			statMorts.set(i, 0.0);
		}
	}
	
	
	/**
	 * Mise ˆ jour de la liste contenant le nombre de malades 
	 * par jour
	 * @param day le numŽro du jour (0..365)
	 * @param nbMalades Nombre de malades 
	 */
	public void saveNbMalades(int day, Double nbMalades){
		statMalades.set(day, nbMalades + statMalades.get(day));
	}

	/**
	 * Mise ˆ jour de la liste contenant le nombre de morts 
	 * @param day le numŽro du jour (0..365)
	 * @param nbMorts Nombre de morts
	 */
	public void saveNbMorts(int day, Double nbMorts){
		statMorts.set(day, nbMorts + statMorts.get(day));
	}
	
	/**
	 * Ajoute le nombre maximum de malades contaminŽs 
	 * ˆ la liste statMaxMalades
	 */
	public void saveMaxMalades() {
		statMaxMalades.add(Utils.getMax(statMalades));
	}

	/**
	 * Ajoute le nombre maximum de morts ˆ la liste
	 * statMaxMorts 
	 */
	public void saveMaxMorts() {
		statMaxMorts.add(Utils.getMax(statMorts));
	}

	
	/**
	 * Calculer la moyenne des morts ˆ une date
	 * prŽcise 
	 */
	public void saveMoyMorts(){
		for (int i = 0; i < nbJours; i++) {
			statMorts.set(i, (statMorts.get(i) / nbExperiences));
		}
	}
	
	
	/**
	 * Sauvegarder la moyenne des malades ˆ une date prŽcise 
	 * dans la liste statMalades
	 */
	public void saveMoyMalades(){
		for (int i = 0; i < nbJours; i++) {
			statMalades.set(i, (statMalades.get(i) / nbExperiences));
		}		
	}
	
	
	/**
	 * Ecriture des statistiques dans des fichiers sur 
	 * le FS.
	 * Ceci concerne la partie dans laquelle aucun 
	 * individu n'est vaccinŽ
	 */
	public void writeStat() {
		try {			
			outMo = new BufferedWriter(new FileWriter("./statistiqueMo.txt"));
			outMo.close();
			outMa = new BufferedWriter(new FileWriter("./statistiqueMa.txt"));
			outMa.close();			
			outMo = new BufferedWriter(new FileWriter("./statistiqueMo.txt",true));
			outMa = new BufferedWriter(new FileWriter("./statistiqueMa.txt",true));
			for (int i = 0; i < nbJours; i++) {
				//outMo.write(i * step + "\t" + (statMorts.get(i) / nbExperiences)  + "\n");
				//outMa.write(i * step + "\t" + (statMalades.get(i) / nbExperiences) + "\n");
				outMo.write(i * step + "\t" + statMorts.get(i)  + "\n");
				outMa.write(i * step + "\t" + statMalades.get(i)  + "\n");
			}
			outMo.close();
			outMa.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * Ecriture des statistiques dans des fichiers sur 
	 * le FS.
	 * Ceci concerne la partie o des individus
	 * sont vaccinŽs
	 */	
	public void writeStatVaccinations() {
		try {

			outMo = new BufferedWriter(new FileWriter("./statistiqueVaccMo.txt"));
			outMo.close();
			outMa = new BufferedWriter(new FileWriter("./statistiqueVaccMa.txt"));
			outMa.close();
			
			outMo = new BufferedWriter(new FileWriter("./statistiqueVaccMo.txt",true));
			outMa = new BufferedWriter(new FileWriter("./statistiqueVaccMa.txt",true));
			for (int i = 0; i < 100; i++) {
				outMo.write(i + "\t" + statMaxMorts.get(i)   + "\n");
				outMa.write(i + "\t" + statMaxMalades.get(i) + "\n");
			}
			outMo.close();
			outMa.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	
}
