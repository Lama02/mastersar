package fr.upmc.sar.psia.tme1;

import java.lang.management.ManagementFactory;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

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
	// Le serveur d'administration
	private MBeanServer mbs = ManagementFactory.getPlatformMBeanServer();
	// Le contexte
	private BundleContext context;
	// Liste des services exporter en JMX
	private Map<ObjectName,Object> services = new HashMap<ObjectName,Object>();

	@Override
	public void start(BundleContext context) throws Exception {
		synchronized (this) {
			System.out.println("[tme1] started");
			this.context = context;
			// Recherche les services deja charger
			ServiceReference[] servicesAlreadyPresent = this.context.getAllServiceReferences(null, null);
			// Parcours les services
			for (int i = 0; i < servicesAlreadyPresent.length; i++) {
				Object obj = this.context.getService(servicesAlreadyPresent[i]);
				String nameClass = obj.getClass().getCanonicalName();
				String nameInterface = nameClass + "MBean";
				Class<?> interfaces[] = obj.getClass().getInterfaces(); // interfaces implementees par le service
				// Verifie si une des interfaces implementees par le service finit bien par la chaine "MBean"
				for (int j=0; j < interfaces.length; j++ ){
					if (interfaces[j].getName().equals(nameInterface)){
						// si implements alors enregistre le MBean
						ObjectName name = new ObjectName(":type=" + nameClass);
						mbs.registerMBean(obj, name);
						services.put(name, obj); // Ajout le service a la Map
						System.out.println("[tme1] Service already present: '" + nameClass + "' REGISTERED");
					}
				}
			}
			this.context.addServiceListener(this);
		}
	}

	@Override
	public void stop(BundleContext arg0) throws Exception {
		synchronized (this) {
			System.out.println("[tme1] stopped");		
			for(Entry<ObjectName, Object> entry : services.entrySet()) {
				ObjectName name = entry.getKey();
				Object obj = entry.getValue();
				if (mbs.isRegistered(name)){
					mbs.unregisterMBean(name); // desabonne le service
					//services.remove(name);     // Supprime le service de la Map
					System.out.println("[tme1] Going to stop...: Service '" + obj.getClass().getCanonicalName() + "' UNREGISTERED");
				}
				// traitements
			}
			mbs      = null;
			services = null;
		}
	}

	@Override
	public void serviceChanged(ServiceEvent ev) {
		synchronized (this) {
			System.out.println("[tme1] New event");

			ServiceReference newRef = ev.getServiceReference();  // la nouvelle reference OSGi
			Object obj = context.getService(newRef);             // objet implementant le service

			String nameClass = context.getService(newRef).getClass().getCanonicalName(); // le nom de la classe du services
			String nameInterface = nameClass + "MBean";          // le nom de l'interface MBean du service

			try {
				ObjectName name = new ObjectName(":type=" + nameClass);	// Le nom associe a la classe a administrer via JMX
				
				switch(ev.getType()) {
				case ServiceEvent.REGISTERED:
					Class<?> interfaces[] = obj.getClass().getInterfaces(); // interfaces implementees par le service
					// Verifie si une des interfaces implementees par le service finit bien par la chaine "MBean"
					for (int i=0; i<interfaces.length; i++ ){
						// si implements et pas deja enregistre alors enregistre le MBean
						if (interfaces[i].getName().equals(nameInterface) && !mbs.isRegistered(name)){
							mbs.registerMBean(obj, name);
							services.put(name, obj);
							System.out.println("[tme1] Service '" + nameClass + "' REGISTERED");
						}
					}
					break;

				case ServiceEvent.UNREGISTERING:
					// si le service est deja enregistre, on le desabonne
					if (mbs.isRegistered(name)){
						mbs.unregisterMBean(name);
						services.remove(name);
						System.out.println("[tme1] Service '" + nameClass + "' UNREGISTERED");
					}
					break;

				case ServiceEvent.MODIFIED: 
					System.out.println("[tme1] Service MODIFIED");
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

}
