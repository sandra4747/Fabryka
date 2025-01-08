#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include "magazyn.h"

// Definicja zmiennych globalnych
Magazyn* magazyn;  // Wskaźnik na magazyn w pamięci dzielonej
int semid;  // Identyfikator semafora

// Funkcja inicjalizująca pamięć dzieloną i semafor
void inicjalizuj_magazyn() {
    key_t key = ftok("magazyn", 'R');  // Tworzenie unikalnego klucza
    int shmid = shmget(key, sizeof(Magazyn), IPC_CREAT | 0666);  // Tworzenie pamięci dzielonej
    magazyn = (Magazyn*) shmat(shmid, NULL, 0);  // Podłączenie pamięci dzielonej

    magazyn->x = 0;
    magazyn->y = 0;
    magazyn->z = 0;
    magazyn->pojemnosc = MAX_POJEMNOSC;
    magazyn->zajeta_przestrzen = 0;

    // Tworzenie semafora
    semid = semget(key, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);  // Ustawienie semafora na 1 (pozwala na dostęp)
}

// Funkcja niszcząca pamięć dzieloną i semafor
void zniszcz_magazyn() {
    shmdt(magazyn);  // Odłączenie pamięci dzielonej
    shmctl(shmget(IPC_PRIVATE, sizeof(Magazyn), 0666), IPC_RMID, NULL);  // Usunięcie pamięci dzielonej
    semctl(semid, 0, IPC_RMID);  // Usunięcie semafora
}

// Funkcja dodająca podzespoły do magazynu
int dodaj_do_magazynu(char* podzespol) {
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;  // Operacja 'wait' na semaforze
    sops.sem_flg = 0;
    semop(semid, &sops, 1);  // Zabezpieczenie dostępu do magazynu

    int zajmowana_przestrzen = 0;

    if (strcmp(podzespol, "X") == 0) {
        zajmowana_przestrzen = 1;  // Podzespół X zajmuje 1 jednostkę
    } else if (strcmp(podzespol, "Y") == 0) {
        zajmowana_przestrzen = 2;  // Podzespół Y zajmuje 2 jednostki
    } else if (strcmp(podzespol, "Z") == 0) {
        zajmowana_przestrzen = 3;  // Podzespół Z zajmuje 3 jednostki
    }

    // Sprawdzamy, czy mamy wystarczająco miejsca w magazynie
    if (magazyn->pojemnosc - magazyn->zajeta_przestrzen < zajmowana_przestrzen) {
        sops.sem_op = 1;  // Operacja 'signal' na semaforze
        semop(semid, &sops, 1);  // Zwolnienie semafora
        return 0;  // Brak miejsca w magazynie
    }

    // Dodajemy podzespół do magazynu
    if (strcmp(podzespol, "X") == 0) {
        magazyn->x++;
    } else if (strcmp(podzespol, "Y") == 0) {
        magazyn->y++;
    } else if (strcmp(podzespol, "Z") == 0) {
        magazyn->z++;
    }

    magazyn->zajeta_przestrzen += zajmowana_przestrzen;  // Aktualizacja zajętej przestrzeni

    // Sprawdzamy, czy magazyn osiągnął pełną pojemność WARUNEK STWORZONY DO TESTOW
    if (magazyn->zajeta_przestrzen == magazyn->pojemnosc) {
        printf("Magazyn osiągnął maksymalną pojemność. Zakończenie programu.\n");
        sops.sem_op = 1;  // Operacja 'signal' na semaforze
        semop(semid, &sops, 1);  // Zwolnienie semafora
        exit(0);  // Zakończenie programu
    }
    
    sops.sem_op = 1;  // Operacja 'signal' na semaforze
    semop(semid, &sops, 1);  // Zwolnienie semafora

    return 1;  // Sukces
}


// Funkcja sprawdzająca poziom dostępnej pojemności
int sprawdz_poziom_pojemnosc() {
    return magazyn->pojemnosc - magazyn->zajeta_przestrzen;
}
