package fr.upmc.sar.psia.tme1;

import java.lang.management.ManagementFactory;

import javax.management.InstanceAlreadyExistsException;
import javax.management.MBeanRegistrationException;
import javax.management.MBeanServer;
import javax.management.MalformedObjectNameException;
import javax.management.NotCompliantMBeanException;
import javax.management.ObjectName;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceEvent;
import org.osgi.framework.ServiceListener;
import org.osgi.framework.ServiceReference;

public class Activator implements BundleActivator, ServiceListener{
	// Serveur d'administration ???
	private MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
	private BundleContext context;

	@Override
	public void start(BundleContext context) throws Exception {
		// TODO Auto-generated method stub
		System.out.println("[tme1] start");
		this.context = context;
		this.context.addServiceListener(this);
	}

	@Override
	public void stop(BundleContext arg0) throws Exception {
		// TODO Auto-generated method stub
		System.out.println("[tme1] stop");		
	}

	@Override
	public void serviceChanged(ServiceEvent ev) {
		// TODO Auto-generated method stub
		ServiceReference newRef = ev.getServiceReference();  // la nouvelle reference OSGi
		System.out.println("[tme1] New event");
		
		try {
			switch(ev.getType()) {
			case ServiceEvent.REGISTERED:
				//TODO
				
				// recuperer le nom du services
				String stringClasses[] = (String[]) newRef.getProperty("objectClass");
				
				// Le nom associe a la classe a administrer
				ObjectName name = new ObjectName(":type=" + stringClasses[0]);
				
				// Ajoute la classe au serveur pour l'administrer
				mbs.registerMBean(context.getService(newRef), name);


			case ServiceEvent.UNREGISTERING:
				//TODO
			case ServiceEvent.MODIFIED: 
				//TODO
			}
		} catch (InstanceAlreadyExistsException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (MBeanRegistrationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NotCompliantMBeanException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (MalformedObjectNameException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NullPointerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
