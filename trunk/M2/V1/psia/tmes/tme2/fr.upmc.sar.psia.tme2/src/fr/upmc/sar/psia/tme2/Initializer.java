package fr.upmc.sar.psia.tme2;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

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
		this.random = new Random();
	}

	@Override
	public boolean execute() {
		System.out.println("Debut de l'intitialisation du reseaux...");
		Grippe.nbMort   = 0.0;
		Grippe.nbMalade = 0.0;

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
			grippe.setNbVoisins();
			grippe.initialierVoisin();
		}

	/*	
		initialiserVoisins2();

		for (int i = 0; i < nbNode; i++) {
			Grippe g = (Grippe) Network.get(i).getProtocol(grippePid);
			System.out.print(g.getNodeId() + ": " );
			for (Grippe v: g.getVoisins()) {
				System.out.print(v.getNodeId() + " ");
			}
			System.out.println("  |  " + g.getNbVoisins());
		}
		
		try {
			Thread.sleep(3000);
		} catch (InterruptedException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		*/
		
		try {
			Set<Integer> setVaccines;
			setVaccines = Utils.createSet(0, nbNode, (int) (Statistique.taux_vacc * nbNode));
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
				for (Grippe dest: grippeEmeteur.getVoisins()) {
					//System.out.println("   Send msg: " + grippeEmeteur.getNodeId() + " -> " + ((Grippe)dest.getProtocol(grippePid)).getNodeId());
					grippeEmeteur.send(msg, dest.getNode());
				}
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		System.out.println("Fin de l'intitialisation du reseaux...");
		return false;
	}


	private void initialiserVoisins2() {
		Set<Grippe> omega = new HashSet<Grippe>();
		for (int i = 0; i < Network.size(); i++) {
			omega.add((Grippe) Network.get(i).getProtocol(grippePid));
		}

		Set<Grippe> sigma = new HashSet<Grippe>();
		Object[] tabSigma = sigma.toArray();


		Object[] tabOmega = omega.toArray();

		while (omega.size() > 0) {
			System.out.println(omega.size());
			
			Grippe u = (Grippe) tabOmega[random.nextInt(tabOmega.length)];
			omega.remove(u);
			tabOmega = omega.toArray();

			Grippe v = (Grippe) tabOmega[random.nextInt(tabOmega.length)];
			omega.remove(v);
			tabOmega = omega.toArray();
/*
			omega.add(u);
			omega.add(v);
			tabOmega = omega.toArray();
*/
			u.getVoisins().add(v);
			v.getVoisins().add(u);
			
			if (u.getVoisins().size() == u.getNbVoisins()) {
				omega.remove(u);
				tabOmega = omega.toArray();
				if (u.getVoisins().size() < Grippe.MAX_VOISIN) {
					sigma.add(u);				
					tabSigma = sigma.toArray();
				}
			}
			else {
				if (u.getVoisins().contains(omega)) {
					if (u.getVoisins().size() >= Grippe.MIN_VOISIN) {
						omega.remove(u);
						tabOmega = omega.toArray();
					} else {
						// TODO:
						Grippe w = (Grippe) tabSigma[random.nextInt(tabSigma.length)];

						u.getVoisins().add(w);
						w.getVoisins().add(u);

						if (w.getVoisins().size() == Grippe.MAX_VOISIN) {
							sigma.remove(w);
							tabSigma = sigma.toArray();	
						}
						omega.add(u);
						tabOmega = omega.toArray();
					}
				}
				else {
					omega.add(u);
					tabOmega = omega.toArray();
				}
			}
			
			//////////////////////////////
			
			if (v.getVoisins().size() == v.getNbVoisins()) {
				omega.remove(v);
				tabOmega = omega.toArray();
				if (v.getVoisins().size() < Grippe.MAX_VOISIN) {
					sigma.add(v);				
					tabSigma = sigma.toArray();
				}
			}
			else {
				if (v.getVoisins().contains(omega)){
					if (v.getVoisins().size() >= Grippe.MIN_VOISIN) {
						omega.remove(v);
						tabOmega = omega.toArray();
					} else {
						// TODO:
						Grippe w = (Grippe) tabSigma[random.nextInt(tabSigma.length)];

						v.getVoisins().add(w);
						w.getVoisins().add(v);

						if (w.getVoisins().size() == Grippe.MAX_VOISIN) {
							sigma.remove(w);
							tabSigma = sigma.toArray();	
						}
						omega.add(v);
						tabOmega = omega.toArray();
					}
				}
				else {
					omega.add(v);
					tabOmega = omega.toArray();
				}
			}

		}

	}


}
