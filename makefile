all: lib assemble

main:
	gcc -g -c main.c -o main.o
	gcc -g main.o pnglib.o -o exe

lib:
	gcc -g -c pnglib.c -o pnglib.o

h:
	gcc -g -c help.c -o help.o

assemble:
	gcc -g main.o pnglib.o help.o -o exe

clean:
	rm *.o

cc: lib main assemble