package fr.upmc.sar.psia.tme2;

import java.util.Random;

import peersim.config.Configuration;
import peersim.core.Control;
import peersim.core.Network;
import peersim.core.Node;

public class Initializer implements Control {

	private static final int NB_INIT_MALADE = 5;

	private int grippePid;

	private Random random;

	public Initializer(String prefix) {
		this.grippePid = Configuration.getPid(prefix + ".helloWorldProtocolPid");
		this.random    = new Random();
	}
	@Override
	public boolean execute() {
		// Le nombre de noeud du reseaux
		int nbNode = Network.size();

		/*
		 *  Pour chaque noeud, on fait le lien entre la couche applicative et la couche transport
		 */
		// Parcours les noeud du reseaux
		for(int i = 0; i < nbNode; i++) {
			// Et pour chaque noeud
			Node node = Network.get(i);
			// Recupere le protocol Grippe via le pid du protocol Grippe
			Grippe grippe = (Grippe) node.getProtocol(grippePid);
			// Intilalise la couche de transport du protocol Grippe
			grippe.setTransportLayer(i);
		}

		for(int i = 0; i < NB_INIT_MALADE; i++) {
			// Le noeud malade
			Node node = Network.get(random.nextInt(Network.size()));
			// Son protocole de Grippe
			Grippe grippeEmeteur = (Grippe) node.getProtocol(grippePid);
			// Le message de la grippe
			Message msg = new Message();
			// Envoi du message au voisins
			for (Node dest: grippeEmeteur.getVoisins()) {
				grippeEmeteur.send(msg, dest);
			}
		}


		return false;
	}

}