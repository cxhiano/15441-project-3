CC = gcc
CFLAGS = -g -Wall  # -DDEBUG -DTESTING=1

all: proxy nameserver

proxy: $(patsubst %.c, %.o, $(wildcard src/proxy/*.c)) \
	   $(patsubst %.c, %.o, $(wildcard src/utils/*.c))
	$(CC) $(CFLAGS) $^ -o proxy

nameserver: $(patsubst %.c, %.o, $(wildcard src/nameserver/*.c)) \
	   		$(patsubst %.c, %.o, $(wildcard src/utils/*.c))
	$(CC) $(CFLAGS) $^ -o nameserver

test: $(patsubst %.c, %.o, $(wildcard src/test/*.c)) \
 	  $(patsubst %.c, %.o, $(wildcard src/utils/*.c))
	$(CC) $(CFLAGS) $^ -o test

clean:
	rm -vf proxy nameserver test
	cd src/proxy; make clean
	cd src/nameserver; make clean
	cd src/test; make clean
	cd src/utils; make clean

tar:
	(make clean; cd ..; cp -r 15441-project-3 handin; tar cvf project3.tar handin; rm -rvf handin)

submit:
	-git tag -d checkpoint-2
	git tag checkpoint-2
	make tar
