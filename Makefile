
all: cvrp

clean:
	rm -f cvrp *.o

cvrp: main.o
	g++ -Ilib -Wall -O3 -ggdb main.o -o cvrp

main.o: main.h main.cpp
	g++ -Ilib -Wall -O3 -ggdb -c main.cpp -o main.o