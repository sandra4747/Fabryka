#ifndef MAGAZYN_H
#define MAGAZYN_H

#include <pthread.h>
#include <sys/sem.h>

#define MAX_POJEMNOSC 100

// Struktura magazynu w pamięci dzielonej
typedef struct {
    int x;  // Ilość podzespołów X
    int y;  // Ilość podzespołów Y
    int z;  // Ilość podzespołów Z
    int pojemnosc;  // Pojemność magazynu
    int zajeta_przestrzen;  // Zajęta przestrzeń w magazynie
} Magazyn;

// Zmienne globalne
extern Magazyn* magazyn;  // Wskaźnik na magazyn w pamięci dzielonej
extern int semid;  // Identyfikator semafora

void inicjalizuj_magazyn();
void zniszcz_magazyn();
int dodaj_do_magazynu(char* podzespol);
int sprawdz_poziom_pojemnosc();

#endif
