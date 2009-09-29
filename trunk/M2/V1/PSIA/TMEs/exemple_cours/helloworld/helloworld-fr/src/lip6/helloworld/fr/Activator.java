

package lip6.helloworld.fr;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;

import lip6.services.helloworld.HelloWorld;

import java.util.Properties;

public class Activator implements BundleActivator {
	public void start(BundleContext context) {
		System.out.println("[HelloWorldFR]: D�marre le helloworld fran�ais");
		HelloWorld hw = new HelloWorldFR();
		Properties props = new Properties();
		props.put("Lang", "Fr");
		context.registerService(HelloWorld.class.getName(), hw, props);
	}

	public void stop(BundleContext context) {
		System.out.println("[HelloWorldFR]: Stop le helloworld fran�ais");
	}
}
