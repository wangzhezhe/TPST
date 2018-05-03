uuid.o:
	g++ -c uuidtest.c -o uuid.o  -luuid 

uuid:uuid.o
	g++ uuid.o -o uuid -luuid 

clean:
	rm uuid uuid.o