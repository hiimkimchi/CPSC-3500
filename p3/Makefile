all: p3

p3: p3.o grid.o
	g++ -o p3 p3.o grid.o -lpthread -std=c++20

p3.o: p3.cpp
	g++ -c p3.cpp -std=c++20

grid.o: grid.h grid.cpp
	g++ -c grid.cpp -std=c++20

clean:   
	rm -rf *.o p3