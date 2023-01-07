CC = gcc
CFLAGS = -g3#-Wall -Werror -Wextra

crossw: crossw.o crossutil.o dict.o actions.o words.o maps.o
	$(CC) $(CFLAGS) crossw.o crossutil.o maps.o words.o actions.o dict.o -o crossw

crossw.o: src/crossw.c
	$(CC) $(CFLAGS) -c src/crossw.c

actions.o: src/actions.c
	$(CC) $(CFLAGS) -c src/actions.c

crossutil.o: src/crossutil.c
	$(CC) $(CFLAGS) -c src/crossutil.c

dict.o: src/dict.c
	$(CC) $(CFLAGS) -c src/dict.c

words.o: src/words.c
	$(CC) $(CFLAGS) -c src/words.c

maps.o: src/maps.c
	$(CC) $(CFLAGS) -c src/maps.c

clean:
	rm -f *.o crossw