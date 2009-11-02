VER=1.0.4

CLASSPATH=.:src:lib/djep-1.0.0.jar:lib/jep-2.3.0.jar:lib/peersim-1.0.5.jar:lib/peersim-doclet.jar

.PHONY: all run clean doc release

all:
	javac -classpath $(CLASSPATH) `find src -name "*.java"`

run:
	java -classpath $(CLASSPATH) peersim/Simulator $(FILE)

clean:
	rm -f `find . -name "*.class"`
	rm -fr bin