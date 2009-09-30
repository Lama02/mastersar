package fr.upmc.sar.psia.tme1;

import java.lang.management.ManagementFactory;

import javax.management.InstanceAlreadyExistsException;
import javax.management.InstanceNotFoundException;
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
		System.out.println("[tme1] started");
		this.context = context;
		this.context.addServiceListener(this);
	}

	@Override
	public void stop(BundleContext arg0) throws Exception {
		// TODO Auto-generated method stub
		System.out.println("[tme1] stopped");		
	}

	@Override
	public void serviceChanged(ServiceEvent ev) {
		// TODO Auto-generated method stub
		ServiceReference newRef = ev.getServiceReference();  // la nouvelle reference OSGi
		System.err.println("[tme1] New event");

		Object obj = context.getService(newRef);                // objet implementant le service

		// recuperer le nom du services
		String nameClass = context.getService(newRef).getClass().getCanonicalName();
		String nameInterface = nameClass + "MBean";

		try {
			// Le nom associe a la classe a administrer
			ObjectName name = new ObjectName(":type=" + nameClass);

			switch(ev.getType()) {
			case ServiceEvent.REGISTERED:
				System.err.println("[tme1] Service REGISTERED");
				Class<?> interfaces[] = obj.getClass().getInterfaces(); // interfaces implementees par le service
				// voir si une des interfaces implementees par le service 
				// finit bien par la chaine "MBean"
				for (int i=0; i<interfaces.length; i++ ){
					if (interfaces[i].getName().equals(nameInterface)){					
						// Ajoute la classe au serveur pour l'administrer
						mbs.registerMBean(obj, name);
					}
				}
				break;

			case ServiceEvent.UNREGISTERING:
				System.err.println("[tme1] Service UNREGISTERED");
				// si le service est deja enregistre, on le desabonne
				if (mbs.isRegistered(name)){
					mbs.unregisterMBean(name);
				}
				break;

			case ServiceEvent.MODIFIED: 
				System.err.println("[tme1] Service MODIFIED");
				//TODO
				break;

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
		} catch (InstanceNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
