all: p1

p1: mcat.o mgrep.o mzip.o munzip.o
	g++ -o mcat mcat.o
	g++ -o mgrep mgrep.o
	g++ -o mzip mzip.o
	g++ -o munzip munzip.o

mcat.o: mcat.cpp
	g++ -c mcat.cpp

mgrep.o: mgrep.cpp
	g++ -c mgrep.cpp

mzip.o: mzip.cpp
	g++ -c mzip.cpp

munzip.o: munzip.cpp
	g++ -c munzip.cpp

clean:   
	rm -rf *.o p1