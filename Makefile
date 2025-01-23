# Kompilator i flagi
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = 

# Pliki źródłowe
SRCS = dostawca.c dyrektor.c monter.c magazyn.c main.c

# Pliki nagłówkowe
HEADERS = magazyn.h dostawca.h dyrektor.h monter.h

# Pliki obiektowe
OBJS = magazyn.o main.o  # Dodajemy tylko główny plik i magazyn.o
DOSTAWCA_OBJS = dostawca.o magazyn.o
DYREKTOR_OBJS = dyrektor.o magazyn.o
MONTER_OBJS = monter.o magazyn.o

# Programy wynikowe
TARGETS = dostawca dyrektor monter fabryka main

# Reguła główna
all: $(TARGETS)

# Kompilacja dostawca
dostawca: $(DOSTAWCA_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja dyrektor
dyrektor: $(DYREKTOR_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja monter
monter: $(MONTER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja fabryka
fabryka: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja main.o (czyli główny program)
main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja magazyn.o
magazyn.o: magazyn.c magazyn.h
	$(CC) $(CFLAGS) -c $< -o $@

# Kompilacja main.o (plik główny)
main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Kompilacja poszczególnych plików obiektowych
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie plików wynikowych
clean:
	rm -f $(TARGETS) *.o

# Phony targets
.PHONY: all clean
