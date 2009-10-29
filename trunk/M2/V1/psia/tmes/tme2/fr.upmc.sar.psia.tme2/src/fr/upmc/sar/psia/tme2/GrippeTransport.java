package fr.upmc.sar.psia.tme2;

import peersim.config.Configuration;
import peersim.core.CommonState;
import peersim.core.Node;
import peersim.core.Protocol;
import peersim.edsim.EDSimulator;

public class GrippeTransport implements Protocol {

	private final long minIncubation;
	private final long rangeIncubation;
	
	private final long minGuerison;
	private final long rangeGuerison;


	public GrippeTransport(String prefix) {
		minIncubation = Configuration.getInt(prefix + ".mindelayIncubation");
		long maxIncubation = Configuration.getInt(prefix + ".maxdelayIncubation");
		if (maxIncubation < minIncubation) {
			System.out.println("The maximum latency cannot be smaller than the minimum latency");
			System.exit(1);
		}
		rangeIncubation = maxIncubation-minIncubation+1;

		minGuerison = Configuration.getInt(prefix + ".mindelayGuerison");
		long maxGuerison = Configuration.getInt(prefix + ".maxdelayGuerison");
		if (maxGuerison < minGuerison) {
			System.out.println("The maximum latency cannot be smaller than the minimum latency");
			System.exit(1);
		}
		rangeGuerison = maxGuerison-minGuerison+1;
	}

	// Envoi d'un message: il suffit de l'ajouter a la file d'evenements
	public void send(Node src, Node dest, Object msg, int pid) {
		long delay;
		
		if (((Message)msg).getType() == Message.MSG_MALADE) {
			delay = getLatencyIncubation(src, dest);
		} else {
			delay = getLatencyGuerison(src, dest);
		}
		
		EDSimulator.add(delay, msg, dest, pid);
	}


	// Latence random entre la borne min et la borne max
	public long getLatencyIncubation(Node src, Node dest) {
		return (rangeIncubation==1?minIncubation:minIncubation + CommonState.r.nextLong(rangeIncubation));
	}
	
	public long getLatencyGuerison(Node src, Node dest) {
		return (rangeGuerison==1?minGuerison:minGuerison + CommonState.r.nextLong(rangeGuerison));
	}


	public Object clone() {
		return this;
	}
}
