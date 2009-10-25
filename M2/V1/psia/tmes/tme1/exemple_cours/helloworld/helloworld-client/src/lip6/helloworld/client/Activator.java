

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
	private boolean          stop;                                // termine le thread associé au bundle
	private BundleContext    context;                             // le context associé au bundle
	private ServiceReference ref;                                 // la référence OSGi courante vers le HelloWorld
	private HelloWorld       hw;                                  // la référence Java courante vers le HelloWorld
	private Thread           self;                                // le thread associé au bundle

	// juste pour afficher des messages
	public void pmsg(String msg) {
		System.out.println("[HelloWorldClient]: " + msg);
	}

	// le thread du bundle
	public void run() {
		try {
			pmsg("Le thread qui dit hello vient de démarrer");
			synchronized(this) {                                      // synchro pour le wait, mais aussi ref et hw 
				while(!stop) {
					if(ref == null)                             					// si aucun service disponible
						wait();                                             //   dort indéfiniement
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

	// libère le service actuellement utilisé et renvoie son ServiceReference
	private ServiceReference unuseService() {
		ServiceReference old = ref;
		if(ref != null) {
			context.ungetService(ref);                                // lâche le service
			ref = null;                                               // indique au thread : aucun service disponible
		}
		return old;
	}

	// assigne un nouveau service au bundle
	private void useService(ServiceReference ref, ServiceReference old) {
		pmsg("Service disponible : (Lang=" + ref.getProperty("Lang") + ")");
		this.ref = ref;                                             // indique au thread : il existe un service courant
		hw = (HelloWorld)context.getService(ref);                   // garde une référence Java vers ce service
		if(old == null)                                             // si on avait pas d'ancien service
			notify();                                                 //    on réveille le thread qui dormait indéfiniement
	}

	// fonction appelée lors d'un changement de service (récéption d'un message de la part du framework)
	public void serviceChanged(ServiceEvent ev) {
		ServiceReference newRef = ev.getServiceReference();         // la nouvelle référence OSGi

		synchronized(this) {                                        // exclue le thread avec useService, unuseService et lookup
			switch(ev.getType()) {
				case ServiceEvent.REGISTERED:                           // si newRef vient d'être enregsitré
					if(ref == null)        				                        //   si on n'avait pas de service
						useService(newRef, ref);                            //      on prend le premier venu
					else if(newRef.getProperty("Lang").equals("Fr") &&    //   si le nouveau est en Fr
									!ref.getProperty("Lang").equals("Fr")) {      //   et si l'ancien n'était pas Fr
						useService(newRef, unuseService());                 //      lâche l'ancien service pour utiliser le nouveau
					}
					break;
				case ServiceEvent.UNREGISTERING:                        // si newRef est en cours de désenregistrement  
					if(newRef == ref)                                     //   et si on l'utilisait
						lookup();                                           //      on lance une nouvelle recherche initiale
					break;
				case ServiceEvent.MODIFIED:                             // si newRef vient de mettre à jour ses propriétés
					if((newRef == ref) &&                                 //   et si c'était lui qu'on utilisait
						 !ref.getProperty("Lang").equals("Fr"))             //   et si il n'est pas en Fr
						lookup();                                           //      on lance un nouvelle recherche car il était peut-être Fr avant
					break;
			}
		}
	}

	// recherche exhaustive de service
	public void lookup() {
		try {
			ServiceReference old = unuseService();                    // old est l'ancienne référence, utilisé dans useService
                                                                // on lâche cette ancienne référence puisqu'on recommence une recherche

			                                                          // Cherche les services HelloWorld (pas de filtre)
			ServiceReference[] refs = context.getServiceReferences(HelloWorld.class.getName(), null);

			if(refs == null)                                          // aucun service disponible actuellement
				pmsg("Aucun service HelloWorld disponible");
			else {
				for(int i=0; i<refs.length && ref==null; i++)           // cherche en priorité un service Fr
					if(refs[i].getProperty("Lang").equals("Fr"))
						useService(refs[i], old);
					
				if(ref == null)                                         // sinon, le premier venu fait l'affaire
					useService(refs[0], old);
			}
		} catch(InvalidSyntaxException e) {
		}
	}

	// démarrage du bundle
	public void start(BundleContext context) throws InvalidSyntaxException {
		pmsg("Démarre le client helloworld");
		pmsg("  Quand un service HelloWorld est présent, invoque sayHello() toutes les 5s");
		pmsg("  Préfère un service avec Lang=Fr à autre chose");
		pmsg("  Ce client s'adapte automatiquement en fonction des services HelloWorld disponibles");
		this.context = context;

		self = new Thread(this, "The simple HelloWorld client");    // cré l'objet Thread associé au bundle
		self.start();                                               // et le démarre

		synchronized(this) {                                        // évite les serviceChanged en // de la recherche initiale
			context.addServiceListener(this,                          // associe this avec les événements de type Service
																 "(&(objectClass=" + HelloWorld.class.getName() + ")" + "(Lang=*))");
			                                                          // filtre : sur les services HelloWorld, quelque soit le Lang

			lookup();                                                 // fait une recherche initiale
		}
	}

	// fin du bundle
	public void stop(BundleContext context) throws InterruptedException {
		pmsg("Stop le client helloworld");

		synchronized(this) {                                        // exclusion avec le thread du bundle pour unuseService, stop, notify()
			unuseService();                                           // lâche la référence
			stop = true;                                              // indique que le programme est terminé
			notify();                                                 // sort le thread de son wait
		}
		self.join();                                                // attend la fin du thread, pas très utile, mais vérifie que tout est ok
		pmsg("Toutes les ressources sont libérées");
	}
}
