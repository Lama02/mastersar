package fr.upmc.sar.psia.tme2;

import peersim.config.Configuration;
import peersim.core.CommonState;
import peersim.core.Node;
import peersim.core.Protocol;
import peersim.edsim.EDSimulator;

public class GrippeTransport implements Protocol {

	private final long min;
	private final long range;


	public GrippeTransport(String prefix) {
		min = Configuration.getInt(prefix + ".mindelay");
		long max = Configuration.getInt(prefix + ".maxdelay");
		if (max < min) {
			System.out.println("The maximum latency cannot be smaller than the minimum latency");
			System.exit(1);
		}
		range = max-min+1;
	}

	// Envoi d'un message: il suffit de l'ajouter a la file d'evenements
	public void send(Node src, Node dest, Object msg, int pid) {
		long delay = getLatency(src,dest);
		EDSimulator.add(delay, msg, dest, pid);
	}


	// Latence random entre la borne min et la borne max
	public long getLatency(Node src, Node dest) {
		return (range==1?min:min + CommonState.r.nextLong(range));
	}


	public Object clone() {
		return this;
	}
}
