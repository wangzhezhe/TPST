
CC=g++
CPPFLAGS=
TARGETS = eventNotify
all: $(TARGETS)

#test-taskmanager:
#	$(CC) -rdynamic -o test-taskmanager ./test-taskmanager.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c

#test-trigure:
#	$(CC) -rdynamic -o test-trigure ./test-registerandtrigger.cpp ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c ../eventstore/eventStore.cpp

#test-json:
#	$(CC) -o test-json ./rapidjsontest.cpp

#test-wn:
#	$(CC) -rdynamic -o notify watchnotifytm.cpp ../lib/file/loaddata.c ../observer/taskmanager.cpp ../observer/observer.cpp ../lib/error/error.c ../eventstore/eventStore.cpp -ldl

eventmanager.o:
	$(CC) -c $(CPPFLAGS) ../src/observer/eventmanager.cpp -o eventmanager.o
loaddata.o: 
	$(CC) -c $(CPPFLAGS) ../src/utils/file/loaddata.c -o loaddata.o
	
eventNotify.o: eventNotify.cpp
	$(CC) -c $(CPPFLAGS) ./eventNotify.cpp

eventNotify: eventNotify.o loaddata.o eventmanager.o
	$(CC) $(CPPFLAGS) ./eventNotify.o ./loaddata.o ./eventmanager.o -lpthread -o eventNotify 

clean:
	rm *.o
	rm eventNotify