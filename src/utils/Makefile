CC = gcc
CFLAGS = -g -Wall

all: $(patsubst %.c, %.o, $(wildcard *.c))

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -vf *.o *.gch
