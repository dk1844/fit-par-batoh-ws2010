mpiCC -c Node.cpp -o Node.o
mpiCC -c main.cpp -o main.o
mpiCC main.o Node.o -o main