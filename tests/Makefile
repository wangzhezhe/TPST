CC := g++

CFLAGS := -Wall -DDEBUG 

all: test-taskmanager test-trigure

test-taskmanager:
	$(CC) -rdynamic -o test-taskmanager ./test-taskmanager.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error.c

test-trigure:
		$(CC) -rdynamic -o test-trigure ./test-registerandtrigger.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error.c ../eventstore/eventStore.cpp

clean:
	rm -f test-taskmanager test-trigure