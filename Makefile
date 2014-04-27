cd src
L=-lm pthread
CC=gcc

main: main.o ae.o anet.o ae_epoll.o zmalloc.o ac.o array.o
	${CC} main.o ae.o anet.o ac.o zmalloc.o array.o -o main ${L}
main.o:main.c
	gcc -c main.c
ae.o:ae.c ae.h
	gcc -c ae.c
anet.o:anet.c anet.h
	gcc -c anet.c
zmalloc.o:
	gcc -c zmalloc.c
ac.o:
	gcc -c ac.c
array.o:
	gcc -c array.c
clean:
	rm -rf ./*.o
	rm -rf main
