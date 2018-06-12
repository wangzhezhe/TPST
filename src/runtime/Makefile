CC=g++ -pthread -Wl,--no-as-needed
CPPFLAGS= -std=c++11 -I../../deps/
TARGETS = test largetest

all: $(TARGETS)

largetest.o: largetest.cc
	$(CC) -c $(CPPFLAGS) ./largetest.cc

local.o: local.cc
	$(CC) -c $(CPPFLAGS) ./local.cc

slurm.o: slurm.c
	$(CC) -c $(CPPFLAGS) ./slurm.c

test.o: test.c
	$(CC) -c $(CPPFLAGS) ./test.c

test: test.o slurm.o 
	$(CC) $(CPPFLAGS) ./test.o ./slurm.o  -o test

largetest: largetest.o local.o 
	$(CC) $(CPPFLAGS) ./largetest.o ./local.o  -o largetest

clean:
	rm *.o
	rm test largetest
	