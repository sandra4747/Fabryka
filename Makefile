all: fabryka

# Główna aplikacja "fabryka"
fabryka: dostawca.o magazyn.o main.o monter.o
	gcc -o fabryka dostawca.o magazyn.o main.o monter.o -lpthread

# Plik obiektowy dla dostawca.c
dostawca.o: dostawca.c dostawca.h magazyn.h
	gcc -c dostawca.c

# Plik obiektowy dla magazyn.c
magazyn.o: magazyn.c magazyn.h
	gcc -c magazyn.c

# Plik obiektowy dla main.c
main.o: main.c dostawca.h magazyn.h
	gcc -c main.c

# Plik obiektowy dla monter.c
monter.o: monter.c magazyn.h monter.h
	gcc -c monter.c

# Czyszczenie plików obiektowych
clean:
	rm -f dostawca.o magazyn.o main.o monter.o fabryka
