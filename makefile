all: lib assemble

main:
	gcc -g -c main.c -o main.o
	gcc -g main.o pnglib.o -o exe

lib:
	gcc -g -c pnglib.c -o pnglib.o

assemble:
	gcc -g main.o pnglib.o -o exe

clean:
	rm *.o

cc: lib main assemble