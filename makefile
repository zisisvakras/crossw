CC = gcc
CFLAGS = -Wall -Werror -Wextra

crossw: crossw.o crossutil.o dict.o
	$(CC) $(CFLAGS) crossw.o crossutil.o dict.o -o crossw

crossw.o: src/crossw.c
	$(CC) $(CFLAGS) -c src/crossw.c

crossutil.o: src/crossutil.c src/crossutil.h
	$(CC) $(CFLAGS) -c src/crossutil.c

dict.o: src/dict.c src/dict.h
	$(CC) $(CFLAGS) -c src/dict.c

clean:
	rm -f *.o crossw