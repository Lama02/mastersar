

package lip6.helloworld.en;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;

import lip6.services.helloworld.HelloWorld;

import java.util.Properties;

public class Activator implements BundleActivator {
	public void start(BundleContext context) {
		System.out.println("[HelloWorldEN]: Start the english helloworld");
		HelloWorld hw = new HelloWorldEN();
		Properties props = new Properties();
		props.put("Lang", "En");
		context.registerService(HelloWorld.class.getName(), hw, props);
	}

	public void stop(BundleContext context) {
		System.out.println("[HelloWorldEN]: Stop the english helloworld");
	}
}
