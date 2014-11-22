CC = gcc
CFLAGS = -g -Wall  # -DDEBUG -DTESTING=1

all: proxy

proxy: $(patsubst %.c, %.o, $(wildcard src/proxy/*.c)) \
	   $(patsubst %.c, %.o, $(wildcard src/utils/*.c))
	$(CC) $(CFLAGS) $^ -o proxy

clean:
	rm -vf proxy
	cd src/proxy; make clean
	cd src/utils; make clean

tar:
	(make clean; cd ..; tar cvf 15-441-project-3.tar 15-441-project-3)

submit:
	-git tag -d checkpoint-1
	git tag checkpoint-1
	make tar
