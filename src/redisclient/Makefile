CC=gcc
CPPFLAGS=-I../../deps/hiredis/ 
TARGETS = test

all: $(TARGETS)

redisclient.o: redisclient.c
	$(CC) -c $(CPPFLAGS) ./redisclient.c

test.o: test.c
	$(CC) -c $(CPPFLAGS) ./test.c

test: test.o redisclient.o
	$(CC) $(CPPFLAGS) ./test.o ./redisclient.o ../../deps/hiredis/*.o -lpthread -o test

clean:
	rm test.o
	rm test 
	