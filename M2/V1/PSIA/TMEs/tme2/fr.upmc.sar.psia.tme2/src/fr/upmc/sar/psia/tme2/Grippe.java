package fr.upmc.sar.psia.tme2;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import peersim.config.Configuration;
import peersim.core.Network;
import peersim.core.Node;
import peersim.edsim.EDProtocol;

public class Grippe implements EDProtocol {

	private static final int NON_IMMUNISER = 0;
	private static final int MALADE        = 1;
	private static final int IMMUNISER     = 2;
	private static final int MORT          = 3;

	// Identifiant de la couche transport
	private int transportPid;
	// La couche de transport
	private GrippeTransport transport;

	// Identifiant du protocol
	private int pid;

	// Identifiant du noeud
	private int nodeId;

	// Le prefix, ici on utilise par le nom du package
	private String prefix;

	// Le random
	private Random random = new Random();

	////////////////////////////////

	// Voisins
	private List<Node> voisins;
	// Etat
	private int etat;

	// Constructeur
	public Grippe(String prefix) {
		this.prefix = prefix;
		this.transportPid = Configuration.getPid(prefix + ".transport");
		this.pid = Configuration.getPid(prefix + ".myself");
		this.transport = null;
		this.voisins = new ArrayList<Node>();
	}

	// Methode appelee lorsqu'un message est recu par le protocole HelloWorld du noeud
	public void processEvent(Node node, int pid, Object event) {
		this.receive((Message)event);
	}

	// Liaison entre un objet de la couche applicative et un objet de la couche transport situes sur le meme noeud
	public void setTransportLayer(int nodeId) {
		// Initialise l'identifiant du noeud
		this.nodeId = nodeId;
		// Initialise le protocole de transport
		this.transport = (GrippeTransport) Network.get(this.nodeId).getProtocol(this.transportPid);
	}

	// Envoi d'un message (l'envoi se fait via la couche transport)
	public void send(Message msg, Node dest) {
		this.transport.send(getMyNode(), dest, msg, pid);
	}

	// Reception d'un message
	public void receive(Message msg) {
		System.out.println("New event...");
		switch (msg.getType()) {
		case Message.MSG_MALADE:
			switch (etat) {
			case NON_IMMUNISER:
				if (random.nextDouble() < 0.25) {
					etat = MALADE;
				}
				break;
			case MALADE:
				break;
			case IMMUNISER:
				if (random.nextDouble() < 0.01) {
					etat = MALADE;
				}
				break;
			}
			break;
		case Message.MSG_GUERIR:
			switch (etat) {
			case NON_IMMUNISER:
				break;
			case MALADE:
				if (random.nextDouble() < 0.02) {
					etat = MORT;
				}
				else {
					etat = IMMUNISER;
				}
				break;
			case IMMUNISER:
				if (random.nextDouble() < 0.01) {
					etat = MALADE;
				}
				break;
			}

			break;
		}
	}

	public List<Node> getVoisins() {
		return voisins;
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
		for (int i = 0; i< 5; i++) {
			this.voisins.add(Network.get(random.nextInt(Network.size())));
		}
	}


}