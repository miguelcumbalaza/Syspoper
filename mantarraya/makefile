CC=gcc

%.o: %.c
	$(CC) -c -o $@ $<

all: prompt

prompt: prompt.o
	gcc -o prompt prompt.o

clean:
	rm -f *.o
