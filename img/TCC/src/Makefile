CC=gcc
CFLAGS=-O2 -lm 
SOURCES=bonds.c  clusters.c  main.c  potentials.c  rings.c  setup.c  tools.c
EXECUTABLE=TCC


all:
	$(CC)  $(CFLAGS) $(SOURCES) -o $(EXECUTABLE) ; mv $(EXECUTABLE) ..

bonds.o: bonds.cpp bonds.h
	$(CC) $(CFLAGS) -c bonds.cpp ; mv $(EXECUTABLE) ..

clusters.o: clusters.cpp clusters.h
	$(CC) $(CFLAGS) -c clusters.cpp ; mv $(EXECUTABLE) ..

potentials.o: potentials.cpp potentials.h
	$(CC) $(CFLAGS) -c potentials.cpp ; mv $(EXECUTABLE) ..

rings.o: rings.cpp rings.h
	$(CC) $(CFLAGS) -c rings.cpp ; mv $(EXECUTABLE) ..

setup.o: setup.cpp setup.h
	$(CC) $(CFLAGS) -c setup.cpp ; mv $(EXECUTABLE) ..

tools.o: tools.cpp tools.h
	$(CC) $(CFLAGS) -c tools.cpp ; mv $(EXECUTABLE) ..

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp  ; mv $(EXECUTABLE) ..
	

clean:
	rm -f *.o TCC
