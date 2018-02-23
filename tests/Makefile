CC := g++

CFLAGS := -Wall -DDEBUG 

all: test-taskmanager test-trigure test-event test-wn

test-taskmanager:
	$(CC) -rdynamic -o test-taskmanager ./test-taskmanager.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c

test-trigure:
	$(CC) -rdynamic -o test-trigure ./test-registerandtrigger.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c ../eventstore/eventStore.cpp

test-json:
	$(CC) -o test-json ./rapidjsontest.cpp

test-wn:
	$(CC) -rdynamic -o notify watchnotifytm.cpp ../lib/file/loaddata.c ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c ../eventstore/eventStore.cpp -ldl

clean:
	rm -f test-taskmanager test-trigure test-json