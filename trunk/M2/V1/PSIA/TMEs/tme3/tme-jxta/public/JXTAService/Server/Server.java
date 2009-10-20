/*
 * Copyright (C) 2003 Gabriel Antoniu and Mathieu Jan 
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

import java.io.FileInputStream;
import java.io.StringWriter;

import net.jxta.document.AdvertisementFactory;
import net.jxta.document.StructuredDocumentFactory;
import net.jxta.document.StructuredTextDocument;
import net.jxta.document.MimeMediaType;
import net.jxta.document.XMLDocument;
import net.jxta.endpoint.Message;
import net.jxta.pipe.PipeService;
import net.jxta.id.IDFactory;
import net.jxta.platform.ModuleClassID;
import net.jxta.pipe.InputPipe;
import net.jxta.peergroup.NetPeerGroupFactory;
import net.jxta.peergroup.PeerGroup;
import net.jxta.protocol.ModuleSpecAdvertisement;
import net.jxta.protocol.ModuleClassAdvertisement;
import net.jxta.protocol.PipeAdvertisement;
import net.jxta.discovery.DiscoveryService;
import net.jxta.exception.PeerGroupException;

public class Server   {    
    static PeerGroup group = null;
    private DiscoveryService discoSvc;
    private PipeService pipeSvc;
    private InputPipe myPipe; // input pipe for the service
    private Message msg;      // message received on input pipe
    private final static String SERVICE = "JXTASPEC:JXTA-EX1"; // service name
    private final static String TAG = "DataTag";               // tag in message
    private final static String FILENAME = "pipeserver.adv";   // file containing
    // pipe advert.
    
    public static void main(String args[]) {
	Server myapp = new Server();
	System.out.println("Starting Service Peer ....");
	myapp.startJxta();
	myapp.startServer();
	myapp.readMessages();
	System.out.println("Good Bye ....");
	System.exit(0);
    }
    
    private void startJxta() {
    	try {
	    System.setProperty( "net.jxta.tls.principal","USERNAME" );
            System.setProperty( "net.jxta.tls.password","PASSWORD" );
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
	    // Create, and Start the default jxta NetPeerGroup
	    ....
	            
	} catch (PeerGroupException e) {
	    // Could not instanciate the group, print the stack and exit
	    System.out.println("fatal error : group creation failure");
	    e.printStackTrace();
	    System.exit(1);
	}
	        
	// Get the discovery, and pipe service
	System.out.println("Getting DiscoveryService");
	discoSvc = ....
	System.out.println("Getting PipeService");
	pipeSvc = ....
    }
    
    private void startServer() {
	System.out.println("Start the Server daemon");
	try {
	            
	    // Create the ModuleClassAdvertisement associated with the service
	    // We build the module class advertisement using the AdvertisementFactory
	    // class by passing it the type of the advertisement we
	    // want to construct. Also set the module spec id based on the module
	    // class id, usin the IDFactory class. The Module class advertisement is a
	    // a very small advertisement that only advertises the existence
	    // of service. In order to access the service, a peer will
	    // have to discover the associated module spec advertisement.
	    ModuleClassAdvertisement mcadv = ....
	            
	    // Init the name, description and id of the module class adv
	    mcadv.setName("JXTAMOD:JXTA-EX1");
	    mcadv.setDescription("Tutorial example to use JXTA module advertisement Framework");
	    ModuleClassID mcID = IDFactory.newModuleClassID();
	    mcadv.setModuleClassID(mcID);
	            
	    // Ok the Module Class advertisement was created, just publish
	    // it in my local cache and to my peergroup (using the discovery service). This
	    // is the NetPeerGroup
	    ....
	            
	    // Create the Module Spec advertisement associated with the service
	    // We build the module Spec Advertisement using the advertisement
	    // Factory class by passing in the type of the advertisement we
	    // want to construct. The Module Spec advertisement will contain
	    // all the information necessary for a client to contact the service
	    // for instance it will contain a pipe advertisement to
	    // be used to contact the service
	            
	    ModuleSpecAdvertisement mdadv = ....
	            
	    // Setup some of the information field about the servive. In this
	    // example, we just set the name, provider and version and a pipe
	    // advertisement. The module creates an input pipes to listen
	    // on this pipe endpoint.
		    
	    mdadv.setName(SERVICE);
	    mdadv.setVersion("Version 1.0");
	    mdadv.setCreator("sun.com");
	    mdadv.setModuleSpecID(IDFactory.newModuleSpecID(mcID));
	    mdadv.setSpecURI("http://www.jxta.org/Ex1");
	            
	    // Create a pipe advertisement for the Service. The client MUST use
	    // the same pipe advertisement to talk to the server. When the client
	    // discovers the module advertisement it will extract the pipe
	    // advertisement to create its pipe. So, we are reading the pipe
	    // advertisement from a default config file to ensure that the
	    // service will always advertise the same pipefs
		    
	    System.out.println("Reading in file " + FILENAME);
	    PipeAdvertisement pipeadv = null;
	            
	    try {
		FileInputStream is = new FileInputStream(FILENAME);
		// create the pipe advertisement from the input stream
		is.close();
	    } catch (java.io.IOException e) {
		System.out.println("failed to read/parse pipe advertisement");
		e.printStackTrace();
		System.exit(-1);
	    }
	            
	    // add the pipe advertisement to the ModuleSpecAdvertisement
	    ....
	            
	    // display the advertisement as a plain text document.
	    System.out.println("Created service advertisement:");
	    StructuredTextDocument doc = (StructuredTextDocument) mdadv.getDocument(new MimeMediaType("text/plain"));
	            
	    StringWriter out = new StringWriter();
	    doc.sendToWriter(out);
	    System.out.println(out.toString());
	    out.close();       
	            
	    // Ok the Module advertisement was created, just publish
	    // it in my local cache and into the NetPeerGroup.
	    ....
	            
	    // We are now ready to start the service --
	    // create the input pipe endpoint (using the pipe service) clients will
	    // use to connect to the service
	    myPipe = ...
	} catch (Exception ex) {
	    ex.printStackTrace();
	    System.out.println("Server: Error publishing the module");
	}
    }
    
    private void readMessages() {
	// Ok no way to stop this daemon, but that's beyond the point
	// of the example!
	while (true) { // loop over every input received from clients
	            
	    System.out.println("Waiting for client messages to arrive");
	            
	    try {
		// Listen on the pipe, waiting for a client message
		msg = ....
	    } catch (Exception e) {
		myPipe.close();
		System.out.println("Server: Error listening for message");
		return;
	    }
	            
	    // Read the message as a String
	    String ip = null;
	            
	    try {    
		// NOTE: The Client and Service have to agree on the tag names.
		// this is part of the Service protocol defined
		// to access the service. Get the message.
		ip = ....
		                
		if (ip != null) {
		    // read the data
		    System.out.println("Server: received message: " + ip);
		                    
		} else
		    System.out.println("Server: error could not find the tag " + TAG);       
	    } catch (Exception e) {
		System.out.println("Server: error receiving message");
	    }            
	}        
    }
}
