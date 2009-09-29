

package lip6.helloworld.client;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceListener;
import org.osgi.framework.ServiceEvent;
import org.osgi.framework.InvalidSyntaxException;

import lip6.services.helloworld.HelloWorld;

import java.util.Properties;

public class Activator implements BundleActivator, Runnable, ServiceListener {
	private boolean          stop;                                // termine le thread associ� au bundle
	private BundleContext    context;                             // le context associ� au bundle
	private ServiceReference ref;                                 // la r�f�rence OSGi courante vers le HelloWorld
	private HelloWorld       hw;                                  // la r�f�rence Java courante vers le HelloWorld
	private Thread           self;                                // le thread associ� au bundle

	// juste pour afficher des messages
	public void pmsg(String msg) {
		System.out.println("[HelloWorldClient]: " + msg);
	}

	// le thread du bundle
	public void run() {
		try {
			pmsg("Le thread qui dit hello vient de d�marrer");
			synchronized(this) {                                      // synchro pour le wait, mais aussi ref et hw 
				while(!stop) {
					if(ref == null)                             					// si aucun service disponible
						wait();                                             //   dort ind�finiement
					else {                                                // sinon
						hw.sayHello();                                      //	 utilise le service courant
						wait(5000);                                         //   et dort 5s
					}
				}
			}
			pmsg("Quitte le thread");
		} catch(Exception e) {
		}
	}

	// lib�re le service actuellement utilis� et renvoie son ServiceReference
	private ServiceReference unuseService() {
		ServiceReference old = ref;
		if(ref != null) {
			context.ungetService(ref);                                // l�che le service
			ref = null;                                               // indique au thread : aucun service disponible
		}
		return old;
	}

	// assigne un nouveau service au bundle
	private void useService(ServiceReference ref, ServiceReference old) {
		pmsg("Service disponible : (Lang=" + ref.getProperty("Lang") + ")");
		this.ref = ref;                                             // indique au thread : il existe un service courant
		hw = (HelloWorld)context.getService(ref);                   // garde une r�f�rence Java vers ce service
		if(old == null)                                             // si on avait pas d'ancien service
			notify();                                                 //    on r�veille le thread qui dormait ind�finiement
	}

	// fonction appel�e lors d'un changement de service (r�c�ption d'un message de la part du framework)
	public void serviceChanged(ServiceEvent ev) {
		ServiceReference newRef = ev.getServiceReference();         // la nouvelle r�f�rence OSGi

		synchronized(this) {                                        // exclue le thread avec useService, unuseService et lookup
			switch(ev.getType()) {
				case ServiceEvent.REGISTERED:                           // si newRef vient d'�tre enregsitr�
					if(ref == null)        				                        //   si on n'avait pas de service
						useService(newRef, ref);                            //      on prend le premier venu
					else if(newRef.getProperty("Lang").equals("Fr") &&    //   si le nouveau est en Fr
									!ref.getProperty("Lang").equals("Fr")) {      //   et si l'ancien n'�tait pas Fr
						useService(newRef, unuseService());                 //      l�che l'ancien service pour utiliser le nouveau
					}
					break;
				case ServiceEvent.UNREGISTERING:                        // si newRef est en cours de d�senregistrement  
					if(newRef == ref)                                     //   et si on l'utilisait
						lookup();                                           //      on lance une nouvelle recherche initiale
					break;
				case ServiceEvent.MODIFIED:                             // si newRef vient de mettre � jour ses propri�t�s
					if((newRef == ref) &&                                 //   et si c'�tait lui qu'on utilisait
						 !ref.getProperty("Lang").equals("Fr"))             //   et si il n'est pas en Fr
						lookup();                                           //      on lance un nouvelle recherche car il �tait peut-�tre Fr avant
					break;
			}
		}
	}

	// recherche exhaustive de service
	public void lookup() {
		try {
			ServiceReference old = unuseService();                    // old est l'ancienne r�f�rence, utilis� dans useService
                                                                // on l�che cette ancienne r�f�rence puisqu'on recommence une recherche

			                                                          // Cherche les services HelloWorld (pas de filtre)
			ServiceReference[] refs = context.getServiceReferences(HelloWorld.class.getName(), null);

			if(refs == null)                                          // aucun service disponible actuellement
				pmsg("Aucun service HelloWorld disponible");
			else {
				for(int i=0; i<refs.length && ref==null; i++)           // cherche en priorit� un service Fr
					if(refs[i].getProperty("Lang").equals("Fr"))
						useService(refs[i], old);
					
				if(ref == null)                                         // sinon, le premier venu fait l'affaire
					useService(refs[0], old);
			}
		} catch(InvalidSyntaxException e) {
		}
	}

	// d�marrage du bundle
	public void start(BundleContext context) throws InvalidSyntaxException {
		pmsg("D�marre le client helloworld");
		pmsg("  Quand un service HelloWorld est pr�sent, invoque sayHello() toutes les 5s");
		pmsg("  Pr�f�re un service avec Lang=Fr � autre chose");
		pmsg("  Ce client s'adapte automatiquement en fonction des services HelloWorld disponibles");
		this.context = context;

		self = new Thread(this, "The simple HelloWorld client");    // cr� l'objet Thread associ� au bundle
		self.start();                                               // et le d�marre

		synchronized(this) {                                        // �vite les serviceChanged en // de la recherche initiale
			context.addServiceListener(this,                          // associe this avec les �v�nements de type Service
																 "(&(objectClass=" + HelloWorld.class.getName() + ")" + "(Lang=*))");
			                                                          // filtre : sur les services HelloWorld, quelque soit le Lang

			lookup();                                                 // fait une recherche initiale
		}
	}

	// fin du bundle
	public void stop(BundleContext context) throws InterruptedException {
		pmsg("Stop le client helloworld");

		synchronized(this) {                                        // exclusion avec le thread du bundle pour unuseService, stop, notify()
			unuseService();                                           // l�che la r�f�rence
			stop = true;                                              // indique que le programme est termin�
			notify();                                                 // sort le thread de son wait
		}
		self.join();                                                // attend la fin du thread, pas tr�s utile, mais v�rifie que tout est ok
		pmsg("Toutes les ressources sont lib�r�es");
	}
}
