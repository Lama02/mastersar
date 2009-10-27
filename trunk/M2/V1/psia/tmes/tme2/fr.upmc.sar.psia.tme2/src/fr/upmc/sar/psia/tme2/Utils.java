package fr.upmc.sar.psia.tme2;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

public class Utils {
	private static  Random random = new Random();
	
	public static Set<Integer> createSet(int debut, int fin, int size) throws Exception {
		Set<Integer> set = new HashSet<Integer>();

		if ((fin - debut + 1) < size) {
			throw new Exception("Create set impossible");
		}

		while (set.size() < size) {
			//System.out.println("size: " + set.size());
			set.add(debut + random.nextInt(fin - debut));
		}

		return set;
	}
}
