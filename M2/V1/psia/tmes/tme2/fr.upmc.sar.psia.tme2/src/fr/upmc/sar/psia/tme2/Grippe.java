package fr.upmc.sar.psia.tme2;

import java.util.Random;
import java.util.Set;
import java.util.HashSet;

import peersim.config.Configuration;
import peersim.core.Network;
import peersim.core.Node;
import peersim.edsim.EDProtocol;

public class Grippe implements EDProtocol {

	private static final int NON_IMMUNISER = 0;
	private static final int MALADE        = 1;
	private static final int IMMUNISER     = 2;
	private static final int MORT          = 3;
	private static final int VACCINER      = 4;

	public static final int MIN_VOISIN    = 8;
	public static final int MAX_VOISIN    = 15;

	public static Double nbMort   = 0.0;
	public static Double nbMalade = 0.0;


	// Identifiant de la couche transport
	private int transportPid;
	// La couche de transport
	private GrippeTransport transport;

	// Identifiant du protocol
	private int pid;

	// Identifiant du noeud
	private int nodeId;
	// Le noeud
	private Node node;

	// Le prefix, ici on utilise par le nom du package
	private String prefix;

	// Le random
	private Random random = new Random();

	////////////////////////////////

	// Voisins
	private Set<Grippe> voisins;
	private Integer nbVoisins;
	// Etat
	private int etat;

	// Constructeur
	public Grippe(String prefix) {
		this.prefix = prefix;
		this.transportPid = Configuration.getPid(prefix + ".transport");
		this.pid = Configuration.getPid(prefix + ".myself");
		this.transport = null;
		this.voisins = new HashSet<Grippe>();
	}

	// Methode appelee lorsqu'un message est recu par le protocole Grippe du noeud
	public void processEvent(Node node, int pid, Object event) {
		this.receive((Message)event);
	}

	// Liaison entre un objet de la couche applicative et un objet de la couche transport situes sur le meme noeud
	public void setTransportLayer(int nodeId) {
		// Initialise l'identifiant du noeud
		this.nodeId = nodeId;
		this.node   = Network.get(this.nodeId);
		// Initialise le protocole de transport
		this.transport = (GrippeTransport) Network.get(this.nodeId).getProtocol(this.transportPid);
	}

	// Envoi d'un message (l'envoi se fait via la couche transport)
	public void send(Message msg, Node dest) {
		this.transport.send(getMyNode(), dest, msg, pid);
	}

	// Reception d'un message
	public void receive(Message msg) {
		//System.out.println("New event...");
		switch (msg.getType()) {
		case Message.MSG_MALADE:
			processMsgMalade();
			break;
		case Message.MSG_GUERIR:
			processMsgGuerir();
			break;
		}
	}

	public Set<Grippe> getVoisins() {
		return voisins;
	}
	
	public Node getNode() {
		return node;
	}
	
	public void setNbVoisins() {
		this.nbVoisins = MIN_VOISIN + random.nextInt(MAX_VOISIN - MIN_VOISIN) + 1;
	}

	public Integer getNbVoisins() {
		return nbVoisins;
	}
	
	//retourne le noeud courant
	private Node getMyNode() {
		return Network.get(this.nodeId);
	}

	// Methode necessaire pour la creation du reseau (qui se fait par clonage d'un prototype)
	public Object clone() {
		return new Grippe(prefix);
	}

	public int getNodeId() {
		return nodeId;
	}
	
	public void initialierVoisin() {
		try {
			int nbNode = Network.size();
			Set<Integer> setVoisins;
			setVoisins = Utils.createSet(0, nbNode, nbVoisins);//MIN_VOISIN + random.nextInt(MAX_VOISIN - MIN_VOISIN) + 1);
			for(Integer i: setVoisins) {
				this.voisins.add((Grippe)Network.get(i).getProtocol(pid));
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}		

	}

	private void processMsgMalade(){
		Message newMsg = null;
		switch (etat) {
		case NON_IMMUNISER:
			newMsg = new Message(Message.MSG_MALADE);
			if (random.nextDouble() < 0.25) {
				synchronized (nbMalade) {nbMalade++;}
				etat = MALADE;
				for (Grippe dest: voisins) {
					send(newMsg, dest.getNode());
				}
				newMsg = new Message(Message.MSG_GUERIR);
				send(newMsg,node);
			}
			break;
		case MALADE:
			break;
		case VACCINER:
			break;
		case IMMUNISER:
			newMsg = new Message(Message.MSG_MALADE);
			if (random.nextDouble() < 0.01) {
				etat = MALADE;
				synchronized (nbMalade) {nbMalade++;}
				for (Grippe dest: voisins) {
					send(newMsg, dest.getNode());
				}
				newMsg = new Message(Message.MSG_GUERIR);
				send(newMsg,node);
			}
			break;
		}
	}

	private void processMsgGuerir(){
		switch (etat) {
		case NON_IMMUNISER:
			break;
		case VACCINER:
			break;
		case MALADE:
			if (random.nextDouble() < 0.02) {
				synchronized (nbMort) {nbMort++;}
				etat = MORT;
			}
			else {
				synchronized (nbMalade) {nbMalade--;}
				etat = IMMUNISER;
			}
			break;
		case IMMUNISER:
			break;
		}
	}
	
	public void vacciner() {
		this.etat = VACCINER;
	}

}
