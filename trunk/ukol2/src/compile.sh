mpiCC -c Node.cpp -o Node.o
mpiCC -c NodeStack.cpp -o NodeStack.o
mpiCC -c main.cpp -o main.o
mpiCC main.o Node.o NodeStack.o -o main