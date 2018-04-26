

CC=g++
CPPFLAGS=
TARGETS = test

all: $(TARGETS)


loaddata.o: loaddata.cpp
	$(CC) -c $(CPPFLAGS) ./loaddata.cpp -o loaddata.o

test.o: test.cpp
	$(CC) -c $(CPPFLAGS) ./test.cpp


test: test.o loaddata.o
	$(CC) $(CPPFLAGS) ./test.o ./loaddata.o -o test

clean:
	rm *.o
	rm test 
	