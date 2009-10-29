package fr.upmc.sar.psia.tme2;

import java.util.Set;

import peersim.config.Configuration;
import peersim.core.Control;
import peersim.core.Network;
import peersim.core.Node;

public class Initializer implements Control {

	private static final int NB_INIT_MALADE = 5;

	private int grippePid;
	
	private double taux_vaccination = Configuration.getDouble("taux_vaccination");

	public Initializer(String prefix) {
		this.grippePid = Configuration.getPid(prefix + ".helloWorldProtocolPid");
	}

	@Override
	public boolean execute() {
		System.out.println("Debut de l'intitialisation du reseaux...");
		Grippe.nbMort   = 0;
		Grippe.nbMalade = 0;

		// Le nombre de noeud du reseaux
		int nbNode = Network.size();

		// Intialisation des noeuds: pour chaque noeud, on fait le lien entre la couche applicative et la couche transport
		// Parcours les noeud du reseaux
		for(int i = 0; i < nbNode; i++) {
			// Et pour chaque noeud
			Node node = Network.get(i);
			// Recupere le protocol Grippe via le pid du protocol Grippe
			Grippe grippe = (Grippe) node.getProtocol(grippePid);
			// Intilalise la couche de transport du protocol Grippe
			grippe.setTransportLayer(i);
			grippe.initialierVoisin();
		}

		try {
			Set<Integer> setVaccines;
			setVaccines = Utils.createSet(0, nbNode, (int) (this.taux_vaccination * nbNode));
			for (Integer i : setVaccines) {
				// Le noeud malade
				Node node = Network.get(i);
				// Son protocole de Grippe
				Grippe grippeVaccine = (Grippe) node.getProtocol(grippePid);
				grippeVaccine.vacciner();
			}
			
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		try {
			Set<Integer> setMalades;
			setMalades = Utils.createSet(0, nbNode, NB_INIT_MALADE);
			// Puis envoi le msg de la grippe par les N premier malade a leur voisins
			for (Integer i : setMalades) {
				// Le noeud malade
				Node node = Network.get(i);
				// Son protocole de Grippe
				Grippe grippeEmeteur = (Grippe) node.getProtocol(grippePid);
				// Le message de la grippe
				Message msg = new Message(Message.MSG_MALADE);
				// Envoi du message au voisins
				for (Node dest: grippeEmeteur.getVoisins()) {
					//System.out.println("   Send msg: " + grippeEmeteur.getNodeId() + " -> " + ((Grippe)dest.getProtocol(grippePid)).getNodeId());
					grippeEmeteur.send(msg, dest);
				}
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.out.println("Fin de l'intitialisation du reseaux...");
		return false;
	}


}
