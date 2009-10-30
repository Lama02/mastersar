/*
 * Copyright (C) 2003 Gabriel Antoniu and Mathieu Jan
 * Revised 2007, Bogdan Nicolae 
 * Project PARIS from IRISA / ENS Cachan Brittany extension
 * All rights reserved.
 *
 * This is a modified version of "Project JXTA: Java Programmer's Guide" examples
 * from Sun Microsystems, Inc for the "Journees Rennaises de formation GRID2+ARP"
 * called "Composants Corba et JXTA" (http://www.irisa.fr/grid2/AnnonceRennes.html)
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the Sun Project JXTA
 * License Version 1.1 (the "License"); you may not use this
 * file except in compliance with the License. A copy of the
 * License is available at http://www.jxta.org/jxta_license.html.
 * ====================================================================
 *
 */

import java.util.Enumeration;

import net.jxta.peergroup.PeerGroup;
import net.jxta.peergroup.NetPeerGroupFactory;
import net.jxta.exception.PeerGroupException;
import net.jxta.discovery.DiscoveryService;
import net.jxta.discovery.DiscoveryListener;
import net.jxta.discovery.DiscoveryEvent;
import net.jxta.document.Advertisement;
import net.jxta.protocol.DiscoveryResponseMsg;
import net.jxta.protocol.PeerAdvertisement;
import net.jxta.rendezvous.RendezVousService;
import net.jxta.rendezvous.RendezvousEvent;
import net.jxta.rendezvous.RendezvousListener;

public class PeerDiscovery implements DiscoveryListener, RendezvousListener {

	private PeerGroup netPeerGroup  = null;
	private DiscoveryService discovery;
	private RendezVousService rendezvous;

	public static void main(String args[]) {
		PeerDiscovery myapp  = new PeerDiscovery();
		myapp.startJxta();
		myapp.run();
	}

	// method to start the JXTA platform
	private void startJxta() {
		try {
			System.setProperty("net.jxta.tls.principal","turbogeek" );
			System.setProperty("net.jxta.tls.password","turbogeek" );
			System.setProperty("net.jxta.logging.Logging", "SEVERE");
		}

		catch ( SecurityException se ) {
			System.err.println( "a security manager exists" );
		}

		catch ( NullPointerException npe ) {
			System.err.println( "key is null." );
		}

		catch ( IllegalArgumentException iae ) {
			System.err.println( "key is empty." );
		}

		try {
			// Create the default NetPeerGroup : use the NetPeerGroupFactory class
			netPeerGroup =  new NetPeerGroupFactory().getInterface();
		}
		catch ( PeerGroupException e) {   
			// could not instantiate the group, print the stack and exit
			System.out.println("fatal error : group creation failure");
			e.printStackTrace();
			System.exit(1);
		}

		// Get the discovery service & rendezvous service from our peer group
		// add this class as listener to both discovery and rendezvous events
		discovery  = netPeerGroup.getDiscoveryService();
		rendezvous = netPeerGroup.getRendezVousService();
		discovery.addDiscoveryListener(this);
		rendezvous.addListener(this);
	}

	/**
	 * This thread loops forever discovering peers
	 * every minute, and displaying the results.
	 */    
	public void run() {
		try {
			while (true) {
				System.out.println("Sending a Discovery Message");
				// send a discovery message looking for other peers
				Advertisement ad = netPeerGroup.getImplAdvertisement();
				discovery.publish(ad);
				// Recherche de tous Peer disponible
				discovery.getRemoteAdvertisements(null, DiscoveryService.PEER, null, null, 0);
				// wait a bit before sending next discovery message
				Thread.sleep(10 * 1000); 
			}
		}
		catch(Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * by implementing DiscoveryListener we must define this method
	 * to deal to discovery responses
	 */
	public void discoveryEvent(DiscoveryEvent event) {
		// Get the event response
		DiscoveryResponseMsg res = event.getResponse();
		
		System.out.println("New event...");
		
		// Get the advertisements from the response
		Enumeration<Advertisement> res_enum = res.getAdvertisements();		
		while (res_enum.hasMoreElements()) {
			// print the peer name from the advertisement
			System.out.println("Peer name: " + ((PeerAdvertisement)res_enum.nextElement()).getName());
		}
	}

	/**
	 * by implementing RendezvousListener we must define this method
	 * to deal with client events
	 */
	public void rendezvousEvent(RendezvousEvent event) {
		// print on stdout which peers connect and reconnect (hint: use the event type)
	}
}
