CC=gcc
CPPFLAGS=
TARGETS = test

all: $(TARGETS)

slurm.o: slurm.c
	$(CC) -c $(CPPFLAGS) ./slurm.c 

test.o: test.c
	$(CC) -c $(CPPFLAGS) ./test.c

test: test.o slurm.o
	$(CC) $(CPPFLAGS) ./test.o ./slurm.o  -o test

clean:
	rm test.o
	rm test 
	