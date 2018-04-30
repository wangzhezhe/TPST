
CC=g++
CPPFLAGS= -std=c++11 -I../deps/  -I../deps/hiredis/ -DTIME -DDEBUG
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


redisclient.o:
	$(CC) -c $(CPPFLAGS) ../src/redisclient/redisclient.c -o redisclient.o

eventmanager.o:
	$(CC) -c $(CPPFLAGS) ../src/observer/eventmanager.cpp -o eventmanager.o

loaddata.o: 
	$(CC) -c $(CPPFLAGS) ../src/utils/file/loaddata.cpp -o loaddata.o
	
slurm.o:
	$(CC) -c $(CPPFLAGS) ../src/runtime/slurm.c -o slurm.o

eventNotify.o: eventNotify.cpp
	$(CC) -c $(CPPFLAGS) ./eventNotify.cpp

eventNotify: eventNotify.o loaddata.o eventmanager.o redisclient.o slurm.o
	$(CC) $(CPPFLAGS) ./redisclient.o ./eventNotify.o ./loaddata.o ./eventmanager.o ./slurm.o ../deps/hiredis/*.o -lpthread -o eventNotify 


clean:
	rm *.o
	rm eventNotify