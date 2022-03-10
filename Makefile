CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g
LOAD=load_balancer
SERVER=server
LB_UTILS=load_balancer_utils

.PHONY: build clean

build: build_t

build_t: main.o $(LOAD).o $(SERVER).o $(LB_UTILS).o Hashtable.o LinkedList.o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(LB_UTILS).o: $(LB_UTILS).c $(LB_UTILS).h
	$(CC) $(CFLAGS) $^ -c

Hashtable.o: Hashtable.c Hashtable.h
	$(CC) $(CFLAGS) $^ -c

LinkedList.o: LinkedList.c LinkedList.h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
