L=-lm -lpthread
CC=gcc
# Default allocator
I = -DUSE_JEMALLOC -I../deps/jemalloc/include
L+= ../deps/jemalloc/lib/libjemalloc.a -ldl

objects =ae.o anet.o zmalloc.o ac.o array.o main.o

all: $(objects)
	${CC} -o main $^ ${L} ${I}

${objects}:%.o:%.c
	${CC} -c -o $@ $< ${I}


.PHONY:clean
clean:
	rm -rf ./*.o
	rm -rf main

#编译依赖库
#jemalloc 内存分配
jemalloc:../deps/jemalloc/lib/libjemalloc.a
	cd ../deps/ && make $@
