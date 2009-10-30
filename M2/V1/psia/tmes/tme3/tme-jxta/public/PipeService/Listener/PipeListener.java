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

import java.io.FileInputStream;

import net.jxta.peergroup.PeerGroup;
import net.jxta.peergroup.NetPeerGroupFactory;
import net.jxta.exception.PeerGroupException;
import net.jxta.document.AdvertisementFactory;
import net.jxta.document.MimeMediaType;
import net.jxta.document.StructuredDocumentFactory;
import net.jxta.document.XMLDocument;
import net.jxta.pipe.PipeService;
import net.jxta.pipe.InputPipe;
import net.jxta.pipe.PipeMsgEvent;
import net.jxta.pipe.PipeMsgListener;
import net.jxta.endpoint.Message;
import net.jxta.endpoint.MessageElement;
import net.jxta.protocol.PipeAdvertisement;

// This application creates an instance of an input pipe,
// and waits for messages on the pipe

public class PipeListener implements PipeMsgListener {
    
    private PeerGroup netPeerGroup = null;
    private final static String TAG = "PipeListenerMsg";
    private final static String FILENAME = "examplepipe.adv";
    
    private PipeService pipeSvc;
    private PipeAdvertisement pipeAdv;
    private InputPipe pipeIn = null;

    public static void main(String args[]) {
        PipeListener myapp = new PipeListener();
        myapp.startJxta();
        myapp.run();
    }
    
    // Starts jxta
    private void startJxta() {
	try {
	    System.setProperty("net.jxta.tls.principal","USERNAME" );
	    System.setProperty("net.jxta.tls.password","PASSWORD" );
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
	    // create, and Start the default jxta NetPeerGroup
	    netPeerGroup =  new NetPeerGroupFactory().getInterface();
		
	} 
	catch (PeerGroupException e) {
	    // could not instantiate the group, print the stack and exit
	    System.out.println("Error: fatal error : group creation failure");
	    e.printStackTrace();
	    System.exit(1);
	}
	// get the pipe service
	pipeSvc = netPeerGroup.getPipeService();
	
	System.out.println("Reading in " + FILENAME);
	try {
	    FileInputStream is = new FileInputStream(FILENAME);
	    // Create the pipeAdvertisement object from the is FileInputStream (use the AdvertisementFactory class)
	    XMLDocument<?> xml = (XMLDocument<?>) StructuredDocumentFactory.newStructuredDocument( MimeMediaType.XMLUTF8, is );
	    pipeAdv = (PipeAdvertisement) AdvertisementFactory.newAdvertisement(xml);

	    is.close();
	} 
	catch (Exception e) {
	    System.out.println("Error: failed to read/parse pipe adv.");
	    e.printStackTrace();
	    System.exit(-1);
	}
    }
    
    // create input pipe, and register as a PipeMsgListener to be 
    // asynchronously notified of any messaged received on this input pipe
    public void run() {	
	try {
	    System.out.println("Creating input pipe");
	    // create the input pipe
	    pipeIn = pipeSvc.createInputPipe(pipeAdv,this);
	} 
	catch (Exception e) {
	    System.out.println("Error creating input pipe.");
	    return;
	}
	if (pipeIn == null) {
	    System.out.println("Error: cannot open InputPipe");
	    System.exit(-1);
	}
	System.out.println("Listener waiting for messages...");
	try {
	    Thread.sleep(Integer.MAX_VALUE);
	} catch (Exception e) {
	    System.out.println("Program interrupted");
	}
    }
    
    // By implementing PipeMsgListener, we define this method to deal with
    // messages as they occur    
    public void pipeMsgEvent ( PipeMsgEvent event ){
	// Get the message object from the event object
	Message msg=null;
	try {
	    msg = event.getMessage();
	    if (msg == null)
		return;
	} 
	catch (Exception e) {
	    e.printStackTrace();
	    return;
	}
	
	// Get the String message element by specifying the element tag
	MessageElement newMessage = msg.getMessageElement(TAG);
	if (newMessage == null)			
	    System.out.println("null msg received");
	else
	    System.out.println("Received message: " + newMessage);
    }
}
