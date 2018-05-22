#
# Bridges - PSC
#
# Intel Compilers are loaded by default; for other compilers please check the module list
#
CC = g++ -std=c11
CFLAGS = -O3
LIBS =

TARGETS = serial
all: $(TARGETS)
serial: serial.o common.o
	$(CC) -o $@ $(LIBS) serial.o common.o

clean:
	rm -f *.o $(TARGETS) *.stdout *.txt
