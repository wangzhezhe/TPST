CC=g++
CPPFLAGS= -std=c++11 -I../../deps/
TARGETS = test

all: $(TARGETS)

slurm.o: slurm.c
	$(CC) -c $(CPPFLAGS) ./slurm.c 

test.o: test.c
	$(CC) -c $(CPPFLAGS) ./test.c

log.o: 
	$(CC) -c $(CPPFLAGS) ../../src/utils/log/log.cpp

test: test.o slurm.o log.o
	$(CC) $(CPPFLAGS) ./test.o ./slurm.o  -o test

clean:
	rm test.o
	rm test 
	