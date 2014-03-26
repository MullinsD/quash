Quash: quash.o
	g++ quash.o -o Quash
quash.o: quash.cpp
	g++ -c quash.cpp
clean:
	rm quash.o
