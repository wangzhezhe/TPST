CC := gcc

CFLAGS := -Wall -DDEBUG 

all: main

main:
	$(CC) -o main main.c ./models/cat.c ./models/dog.c ../observer/observer.c  ../observer/subject.c

clean:
	rm -f main