# Kompilator i flagi
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = 

# Pliki źródłowe
SRCS = dostawca.c dyrektor.c monter.c magazyn.c
HEADERS = dostawca.h dyrektor.h monter.h magazyn.h

# Pliki obiektowe
OBJS = magazyn.o
DOSTAWCA_OBJS = dostawca.o $(OBJS)
DYREKTOR_OBJS = dyrektor.o $(OBJS)
MONTER_OBJS = monter.o $(OBJS)

# Programy wynikowe
TARGETS = dostawca dyrektor monter

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

# Kompilacja magazyn.o (bez maina)
magazyn.o: magazyn.c magazyn.h
	$(CC) $(CFLAGS) -c $< -o $@

# Kompilacja poszczególnych plików obiektowych
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie plików wynikowych
clean:
	rm -f $(TARGETS) *.o

# Phony targets
.PHONY: all clean