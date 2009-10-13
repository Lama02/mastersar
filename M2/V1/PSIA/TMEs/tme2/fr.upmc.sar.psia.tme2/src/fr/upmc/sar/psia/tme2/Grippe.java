package fr.upmc.sar.psia.tme2;

import java.util.ArrayList;
import java.util.List;

import peersim.config.Configuration;
import peersim.core.Network;
import peersim.core.Node;
import peersim.edsim.EDProtocol;

public class Grippe implements EDProtocol {


	// Identifiant de la couche transport
	private int transportPid;
	// La couche de transport
	private GrippeTransport transport;

	// Identifiant du protocol
	private int pid;

	// Identifiant du noeud
	private int nodeId;
	
	// Voisins
	private List<Node> voisins;


	// Le prefix, ici on utilise par le nom du package
	private String prefix;


	public Grippe(String prefix) {
		this.prefix = prefix;
		this.transportPid = Configuration.getPid(prefix + ".transport");
		this.pid = Configuration.getPid(prefix + ".myself");
		this.transport = null;
		this.voisins = new ArrayList<Node>();
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

	public void processEvent(Node arg0, int arg1, Object arg2) {
		// TODO Auto-generated method stub

	}
	
	public List<Node> getVoisins() {
		return voisins;
	}

	// Methode necessaire pour la creation du reseau (qui se fait par clonage d'un prototype)
	public Object clone() {
		return new Grippe(prefix);
	}

	//retourne le noeud courant
	private Node getMyNode() {
		return Network.get(this.nodeId);
	}
}
