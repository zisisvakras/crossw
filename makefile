CC = gcc
CFLAGS = -g3#-Wall -Werror -Wextra

crossw: crossw.o crossutil.o dict.o
	$(CC) $(CFLAGS) crossw.o crossutil.o dict.o -o crossw

crossw.o: src/crossw.c src/library.h
	$(CC) $(CFLAGS) -c src/crossw.c

crossutil.o: src/crossutil.c src/library.h
	$(CC) $(CFLAGS) -c src/crossutil.c

dict.o: src/dict.c src/library.h
	$(CC) $(CFLAGS) -c src/dict.c

clean:
	rm -f *.o crossw