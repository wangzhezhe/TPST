CC=gcc
CPPFLAGS=-I../
TARGETS = example

all: $(TARGETS)

example.o: example.c
	$(CC) -c $(CPPFLAGS) ./example.c

example: example.o
	$(CC) $(CPPFLAGS) ./example.o ../*.o -o example

clean:
	rm .example.o
	rm .example 
	