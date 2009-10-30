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
import java.io.IOException;

import net.jxta.peergroup.NetPeerGroupFactory;
import net.jxta.peergroup.PeerGroup;
import net.jxta.exception.PeerGroupException;
import net.jxta.document.AdvertisementFactory;
import net.jxta.document.MimeMediaType;
import net.jxta.document.StructuredDocumentFactory;
import net.jxta.document.XMLDocument;
import net.jxta.pipe.PipeService;
import net.jxta.pipe.OutputPipe;
import net.jxta.pipe.OutputPipeListener;
import net.jxta.pipe.OutputPipeEvent;
import net.jxta.protocol.PipeAdvertisement;
import net.jxta.endpoint.Message;
import net.jxta.endpoint.StringMessageElement;

// This example illustrates how to use the OutputPipeListener interface
public class PipeSender implements OutputPipeListener {    
    private PeerGroup netPeerGroup = null;
    private final static String TAG = "PipeListenerMsg";
    private final static String FILENAME = "examplepipe.adv";
    
    private PipeService pipeSvc;
    private PipeAdvertisement pipeAdv;
    
    public static void main(String args[]) {
	PipeSender myapp = new PipeSender();
	myapp.startJxta();
	myapp.run();
    }

    // Starts jxta, and gets the pipe service    
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
	    // create and start the default jxta NetPeerGroup
	    netPeerGroup =  new NetPeerGroupFactory().getInterface();
	} 
	catch (PeerGroupException e) {
	    // could not instantiate the group, print the stack and exit
	    System.out.println("fatal error : group creation failure");
	    e.printStackTrace();
	    System.exit(-1);
	}
		
	// Get the pipe service
    pipeSvc = netPeerGroup.getPipeService();
	
	System.out.println("Reading in from file " + FILENAME);
	try {
	    FileInputStream is = new FileInputStream(FILENAME);
	    // Create the pipeAdvertisement object from the is FileInputStream (use the AdvertisementFactory class)
	    XMLDocument<?> xml = (XMLDocument<?>) StructuredDocumentFactory.newStructuredDocument( MimeMediaType.XMLUTF8, is );
	    pipeAdv = (PipeAdvertisement) AdvertisementFactory.newAdvertisement(xml);

	    is.close();
	} 
	catch (Exception e) {
	    System.out.println("failed to read/parse pipe advertisement");
	    e.printStackTrace();
	    System.exit(-1);
	}
    }
    
    public void run() { 
	while (true) try {
		System.out.println("Sending message to listener...");
		// create the output pipe using the pipe service and the pipe advertisement
		pipeSvc.createOutputPipe(pipeAdv, this);

		Thread.sleep(10 * 1000);
	    } catch (Exception e) {
		System.out.println("Message sending exception: ");
		e.printStackTrace();
	    }
    }
    
    // by implementing OutputPipeListener we must define this method
    // which is called when the output pipe is created  
    public void outputPipeEvent(OutputPipeEvent event) {
	System.out.println("Got an output pipe event");
	
	// Getting the output pipe
	OutputPipe op = event.getOutputPipe();
	String myMsg = "Hello from peer from AC Team " + netPeerGroup.getPeerName();
		
	try {
	    // Create a message object, add a (tag,value) pair to the message, 
	    // then send the message using the pipe
	    Message msg = new Message();
		StringMessageElement sme = new StringMessageElement(TAG, myMsg, null);
		msg.addMessageElement(sme);
		
		op.send(msg);
	} 
	catch (IOException e) {
	    System.out.println("Error: failed to send message");
	    e.printStackTrace();
	    System.exit(-1);
	}
	// Close the pipe
	op.close();
	System.out.println("Message successfully sent.");
    }
}
