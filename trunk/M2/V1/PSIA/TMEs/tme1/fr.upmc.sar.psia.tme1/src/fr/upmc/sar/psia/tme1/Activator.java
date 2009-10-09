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
	private Map<String,ObjectName> services = new HashMap<String,ObjectName>();

	@Override
	public void start(BundleContext context) throws Exception {
		synchronized(this) {
			System.out.println("[tme1] Started");
			this.context = context;
			// Recherche les services deja charge
			ServiceReference[] servicesAlreadyPresent = this.context.getAllServiceReferences(null, null);
			// Parcours les services
			System.out.println("[tme1] Service already present, going to be REGISTERED...");
			for (int i = 0; i < servicesAlreadyPresent.length; i++) {
				// Et les enregistrent si possible
				registerService(servicesAlreadyPresent[i]);
			}
			System.out.println("[tme1] ... service already present finish");
			this.context.addServiceListener(this);
		}
	}

	@Override
	public void stop(BundleContext arg0) throws Exception {
		synchronized(this) {	
			// Parcours les services deja expose
			for(Entry<String, ObjectName> entry : services.entrySet()) {
				String key = entry.getKey();
				ObjectName value = entry.getValue();
				// Et les  desenregistre si possible
				if (mbs.isRegistered(value)){
					mbs.unregisterMBean(value);
					System.out.println("   [tme1] Service '" + key + "' UNREGISTERED");
				}
			}
			mbs      = null;
			services = null;
			System.out.println("[tme1] Stopped");	
		}
	}

	@Override
	public void serviceChanged(ServiceEvent ev) {
		synchronized(this) {
			System.out.println("[tme1] New event");

			ServiceReference newRef = ev.getServiceReference();  // la nouvelle reference OSGi

			switch(ev.getType()) {
			case ServiceEvent.REGISTERED:
				registerService(newRef);
				break;

			case ServiceEvent.UNREGISTERING:
				unregisterService(newRef);
				break;

			case ServiceEvent.MODIFIED: 
				unpdateRegistedService(newRef);
				break;

			}
		}
	}

	private void registerService(ServiceReference newRef) {
		synchronized(this) {
			try {
				// L'objet implementant le service
				Object obj = context.getService(newRef);

				// Recupere les proprietes du services
				String[] keyProps  = newRef.getPropertyKeys();
				String strProps    = "";
				for (int k = 0; k < keyProps.length; k++) {
					if (newRef.getProperty(keyProps[k]) instanceof String) {
						strProps += "," + keyProps[k] + "=" + ((String) newRef.getProperty(keyProps[k]));
					}
				}

				// Le nom de la classe du services
				String nameClass = obj.getClass().getCanonicalName();

				// Le nom de l'interface MBean que devervait implementer le service
				String nameInterface = nameClass + "MBean";

				// Le ObjectName associe a la classe a administrer via JMX
				ObjectName objectName = new ObjectName(":type=" + nameClass + strProps);

				// Les interfaces implementees par le service
				Class<?> interfaces[] = obj.getClass().getInterfaces();

				// Verifie si une des interfaces implementees par le service finit bien par la chaine "MBean"
				for (int i=0; i<interfaces.length; i++ ){
					// si implements et pas deja enregistre alors enregistre le MBean
					if (interfaces[i].getName().equals(nameInterface) && !mbs.isRegistered(objectName)){
						mbs.registerMBean(obj, objectName);
						services.put(nameClass, objectName);
						System.out.println("   [tme1] Service '" + nameClass + "' REGISTERED");
					}
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

	private void unregisterService(ServiceReference newRef) {
		synchronized(this) {
			try {
				// L'objet implementant le service
				Object obj = context.getService(newRef);

				// Le nom de la classe du services
				String nameClass = obj.getClass().getCanonicalName(); 

				// Le ObjectName associe a la classe a administrer via JMX
				ObjectName objectName = services.get(nameClass);	

				// Si le service est deja enregistre, on le desabonne
				if ((objectName != null) && mbs.isRegistered(objectName)){
					mbs.unregisterMBean(objectName);
					services.remove(objectName);
					System.out.println("   [tme1] Service '" + nameClass + "' UNREGISTERED");
				}
			} catch (MBeanRegistrationException e) {
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

	private void unpdateRegistedService(ServiceReference newRef) {

		try {
			// L'objet implementant le service
			Object obj = context.getService(newRef);

			// Recupere les proprietes du services
			String[] keyProps  = newRef.getPropertyKeys();
			String strProps    = "";
			for (int k = 0; k < keyProps.length; k++) {
				if (newRef.getProperty(keyProps[k]) instanceof String) {
					strProps += "," + keyProps[k] + "=" + ((String) newRef.getProperty(keyProps[k]));
				}
			}

			// Le nom de la classe du services
			String nameClass = obj.getClass().getCanonicalName();

			// Le nouveau ObjectName associe a la classe a administrer via JMX
			ObjectName newObjectName = new ObjectName(":type=" + nameClass + strProps);
			// L'ancien   ObjectName associe a la classe a administrer via JMX
			ObjectName oldObjectName = services.get(nameClass);

			// Si le service etait deja enregistrer alors maj sinon on ne fais rien
			if (mbs.isRegistered(oldObjectName)){
				// Desenregistre l'ancien   MBean
				mbs.unregisterMBean(oldObjectName);
				// Resenregistre le nouveau MBean
				mbs.registerMBean(obj,newObjectName);
				// Met a jours la HashMap
				services.put(nameClass, newObjectName);
				System.out.println("   [tme1] Service '" + nameClass + "' MODIFIED");
			}
		} catch (InstanceNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (MalformedObjectNameException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NullPointerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (MBeanRegistrationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InstanceAlreadyExistsException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NotCompliantMBeanException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 

	}

}
