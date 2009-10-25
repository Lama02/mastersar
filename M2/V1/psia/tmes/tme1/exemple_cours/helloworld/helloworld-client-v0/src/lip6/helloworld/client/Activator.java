

package lip6.helloworld.client.v0;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceListener;
import org.osgi.framework.ServiceEvent;
import org.osgi.framework.InvalidSyntaxException;

import lip6.services.helloworld.HelloWorld;

import java.util.Properties;

public class Activator implements BundleActivator {
	// juste pour afficher des messages
	public void pmsg(String msg) {
		System.out.println("[HelloWorldClient - version initiale]: " + msg);
	}

	// démarrage du bundle
	public void start(BundleContext context) throws InvalidSyntaxException {
		pmsg("Démarre le client");
		pmsg("  Ce client cherche un HelloWorld en Fr et invoque une unique fois sayHello() dessus");

		// recherche un helloworld en français
		ServiceReference[] refs = context.getServiceReferences(HelloWorld.class.getName(), "(Lang=Fr)");

		if(refs == null)                                          // aucun service disponible actuellement
			pmsg("Aucun service HelloWorld Fr disponible");
		else {
			HelloWorld hw = (HelloWorld)context.getService(refs[0]);// trouve la référence Java à partir de la référence OSGi
			hw.sayHello();                                          // dit bonjour
			context.ungetService(refs[0]);                          // et informe OSGi qu'on ne se sert plus de cette ref
		}
	}

	// fin du bundle
	public void stop(BundleContext context) throws InterruptedException {
		pmsg("Stop le client");
	}
}
