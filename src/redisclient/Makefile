CC=gcc
CPPFLAGS=-I../../deps/hiredis/ 
TARGETS = test

all: $(TARGETS)

#redisclient.o: redisclient.c
#	$(CC) -c $(CPPFLAGS) ./redisclient.c 

test.o: test.c
	$(CC) -c $(CPPFLAGS) ./test.c

slurm.o: ../runtime/slurm.c
	$(CC) -c $(CPPFLAGS) ../runtime/slurm.c

test: test.o slurm.o
	$(CC) $(CPPFLAGS) ./test.o ./slurm.o ../../deps/hiredis/*.o -lpthread -o test

clean:
	rm *.o
	rm test 
	
	