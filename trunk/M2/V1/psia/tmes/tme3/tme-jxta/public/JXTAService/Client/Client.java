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

import java.io.IOException;
import java.io.StringWriter;
import java.util.Enumeration;

import net.jxta.document.StructuredTextDocument;
import net.jxta.document.MimeMediaType;
import net.jxta.endpoint.Message;
import net.jxta.endpoint.StringMessageElement;
import net.jxta.pipe.PipeService;
import net.jxta.pipe.OutputPipe;
import net.jxta.protocol.ModuleSpecAdvertisement;
import net.jxta.protocol.PipeAdvertisement;
import net.jxta.discovery.DiscoveryService;
import net.jxta.peergroup.NetPeerGroupFactory;
import net.jxta.peergroup.PeerGroup;
import net.jxta.exception.PeerGroupException;

public class Client {
    static PeerGroup netPeerGroup = null;
    private DiscoveryService discoSvc;
    private PipeService pipeSvc;
    
    private OutputPipe myPipe; // Output pipe to connect the service
    private Message msg;       // message to be sent
    private final static String SERVICE = "JXTASPEC:JXTA-EX1"; // service name
    private final static String TAG = "DataTag"; // tag in message
    
    public static void main(String args[]) {
        Client myapp = new Client();
        System.out.println("Starting Client peer ....");
        myapp.startJxta();
        myapp.startClient();
        System.out.println("Good Bye ....");
        System.exit(0);
    }
    
    public Client() { }
    
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
        } 
        catch (PeerGroupException e) {
            // Could not instantiate the group, print the stack and exit
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
    
    // start the client
    private void startClient() {
        // Let's initialize the client
        System.out.println("Start the Client");
        // Let's try to locate the service advertisement SERVICE
        // we will loop until we find it!
        System.out.println("searching for the " + SERVICE + " service advertisement");
        Enumeration enumer = null;
        while (true) {
            try {
                // let's look first in our local cache to see
                // if we have it! We try to discover an advertisement
                // which has the (Name, JXTASPEC:JXTA-EX1) tag value
                //
                enumer = ....
                
                // Found it! Stop searching and go send a message.
                if ((enumer != null) && enumer.hasMoreElements()) break;
                
                // We could not find anything in our local cache, so let's send a
                // remote discovery request searching for the service advertisement
                ....
                
                // The discovery is asynchronous as we do not know
                // how long is going to take
                try { // sleep as much as we want. Yes we
                    // could implement asynchronous listener pipe...
                    Thread.sleep(2000);
                } 
                catch (Exception e) {
                }
                
            } 
            catch (IOException e) {
                // found nothing!  move on
            }
            System.out.print(".");
        }
        
        System.out.println("We found the service advertisement:");
        
        // Ok get the service advertisement as a ModuleSpecAdvertisement
        ModuleSpecAdvertisement mdsadv = (ModuleSpecAdvertisement) enumer.nextElement();
        try {
	    // get the advertisement as a plain text document form the spec advertisement. Use MimeMediaType "text/plain".
            StructuredTextDocument doc = ...

            // let's print the advertisement as a plain text document
            StringWriter out = new StringWriter();
            doc.sendToWriter(out);
            System.out.println(out.toString());
            out.close();
            
            // Get the pipe advertisement from the module spec advertisement -- need it to talk to the service
            PipeAdvertisement pipeadv = ....
            
            if (pipeadv == null){
                System.out.println("Error -- Null pipe advertisement!");
                System.exit(1);
            }
            
	    // create the output pipe endpoint to connect
	    // to the server, try 3 times to bind the pipe endpoint to
	    // the listening endpoint pipe of the service. Use a timeout, not an asynchronous event
	    myPipe = null;
	    for (int i=0; i<3; i++) {        
		System.out.println("Trying to bind to pipe...");
		try {
		    myPipe = ....
		    break;
		} catch (java.io.IOException e) {
		    // go try again;
		}
	    }
	    if (myPipe == null) {
		System.out.println("Error resolving pipe endpoint");
		System.exit(1);
	    }                        
	    // create the data string to send to the server
	    String data = "Hello my friend!";
	            
	    // create the pipe message and add the data with tag TAG
	    ....
	    // send the message to the service pipe and close the pipe
	    ....
	    System.out.println("message \"" + data + "\" sent to the Server");	            
	} catch (Exception ex) {
	    ex.printStackTrace();
	    System.out.println("Client: Error sending message to the service");
	}        
    }
}
