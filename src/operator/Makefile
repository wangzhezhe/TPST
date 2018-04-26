CC=g++
CPPFLAGS=-I../../deps/hiredis/
TARGETS = operator

all: $(TARGETS)


redisclient.o: 
	$(CC) -c $(CPPFLAGS) ../redisclient/redisclient.c -o redisclient.o

operator.o: operator.cpp
	$(CC) -c $(CPPFLAGS) ./operator.cpp

operator: operator.o redisclient.o
	$(CC) $(CPPFLAGS) ./operator.o ./redisclient.o ../../deps/hiredis/*.o -o operator

clean:
	rm ./*.o
	rm ./operator