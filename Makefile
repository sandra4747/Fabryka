all: fabryka

# Główna aplikacja "fabryka"
fabryka: dostawca.o magazyn.o
	gcc -o fabryka dostawca.o magazyn.o -lpthread

# Plik obiektowy dla dostawca.c
dostawca.o: dostawca.c magazyn.h
	gcc -c dostawca.c

# Plik obiektowy dla magazyn.c
magazyn.o: magazyn.c magazyn.h
	gcc -c magazyn.c

