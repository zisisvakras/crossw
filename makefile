CC = gcc
CFLAGS = -g3#-Wall -Werror -Wextra

crossw: crossw.o crossutil.o dict.o words.o
	$(CC) $(CFLAGS) crossw.o crossutil.o words.o dict.o -o crossw

crossw.o: src/crossw.c
	$(CC) $(CFLAGS) -c src/crossw.c

crossutil.o: src/crossutil.c
	$(CC) $(CFLAGS) -c src/crossutil.c

dict.o: src/dict.c
	$(CC) $(CFLAGS) -c src/dict.c

words.o: src/words.c
	$(CC) $(CFLAGS) -c src/words.c

clean:
	rm -f *.o crossw