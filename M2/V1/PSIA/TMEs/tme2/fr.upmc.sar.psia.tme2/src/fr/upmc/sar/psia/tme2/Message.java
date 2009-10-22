package fr.upmc.sar.psia.tme2;

public class Message {
	private int type;
	
	public static final int MSG_MALADE = 0;
	public static final int MSG_GUERIR = 1;

	public Message(int type) {
		this.type = type;
	}
	
	public int getType() {
		return type;
	}
}
