
package lip6.helloworld.en;

import lip6.services.helloworld.HelloWorld;

public class HelloWorldEN implements HelloWorldENMBean {
	public void sayHello() {
		System.out.println("[HelloWorldEN]: Hello, World!");
	}
}