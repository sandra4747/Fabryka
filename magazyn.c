#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "magazyn.h"


void sem_op(int semid, int semnum, int op) {
    struct sembuf operation = {semnum, op, 0};
    semop(semid, &operation, 1);
}

void cleanup(int semid, int shmid, SharedMemory *shm) {
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

int is_magazyn_full(SharedMemory *shm) {
    // Sprawdź, czy w sekcji X nie ma już wolnego miejsca
    for (int i = 0; i < MAX_SPACE / 6; i += UNIT_SIZE_X) {
        if (shm->magazyn[i] == '\0') {
            return 0;  // Magazyn niepełny (znaleziono wolne miejsce)
        }
    }

    // Sprawdź, czy w sekcji Y nie ma już wolnego miejsca
    for (int i = MAX_SPACE / 6; i < MAX_SPACE / 2; i += UNIT_SIZE_Y) {
        if (shm->magazyn[i] == '\0') {
            return 0;  // Magazyn niepełny (znaleziono wolne miejsce)
        }
    }

    // Sprawdź, czy w sekcji Z nie ma już wolnego miejsca
    for (int i = MAX_SPACE / 2; i < MAX_SPACE; i += UNIT_SIZE_Z) {
        if (shm->magazyn[i] == '\0') {
            return 0;  // Magazyn niepełny (znaleziono wolne miejsce)
        }
    }

    return 1;  // Magazyn pełny (brak wolnych miejsc)
}

int is_magazyn_empty(SharedMemory *shm) {
    // Iteracja po wszystkich elementach magazynu
    for (int i = 0; i < MAX_SPACE; i++) {
        if (shm->magazyn[i] != '\0') {
            return 0;  // Magazyn nie jest pusty (znaleziono element)
        }
    }
    return 1;  // Magazyn jest pusty
}