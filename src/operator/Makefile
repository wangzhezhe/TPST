CC=g++
CPPFLAGS=-I../../deps/hiredis/
TARGETS = operator

all: $(TARGETS)

operator.o: operator.cpp
	$(CC) -c $(CPPFLAGS) ./operator.cpp

operator: operator.o
	$(CC) $(CPPFLAGS) ./operator.o ../../deps/hiredis/*.o -o operator

clean:
	rm ./operator