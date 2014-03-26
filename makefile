Quash: quash.o
	g++ quash.o -o Quash
	rm quash.o
quash.o: quash.cpp
	g++ -c quash.cpp
