CC = gcc
CFLAGS = -g -Wall

all: $(patsubst %.c, %.o, $(wildcard *.c))

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	cd utils; make clean
	rm -vf *.o *.gch
