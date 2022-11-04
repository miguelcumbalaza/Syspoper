CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: miniecs_server

miniecs_server: miniecs_server.o
	gcc -pthread -o miniecs_server miniecs_server.o
	
clean:
	rm -f *.o
